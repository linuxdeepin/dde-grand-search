#!/usr/bin/env bash
# SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
#
# SPDX-License-Identifier: GPL-3.0-or-later

# Perf Flame Graph Profiling Script
#
# A guided wrapper around `perf record` + Brendan Gregg's FlameGraph toolchain
# to collect CPU samples from a running process (or a launched command) and
# generate an interactive SVG flame graph.
#
# Usage:
#     ./perf_flamegraph.sh [options]
#
# Common examples:
#     ./perf_flamegraph.sh                            # profile dde-grand-search-daemon for 30s
#     ./perf_flamegraph.sh --program dde-grand-search # profile the frontend process
#     ./perf_flamegraph.sh --duration 60 --freq 999   # 60s at 999Hz
#     ./perf_flamegraph.sh --pid 12345                # profile a specific PID
#     ./perf_flamegraph.sh --command "/usr/bin/dde-grand-search-daemon"  # launch & profile
#     ./perf_flamegraph.sh --interactive              # step-by-step guided mode
#
# Options:
#     -p, --pid PID               Attach to an existing PID (overrides --program)
#         --program NAME          Find PID by program name (default: dde-grand-search-daemon)
#         --command CMD           Launch CMD under perf instead of attaching
#         --freq N                Sampling frequency in Hz (default: 99)
#         --call-graph MODE       Unwinding method: dwarf|fp (default: dwarf)
#     -d, --duration SEC          Sampling duration; 0 = until Ctrl+C (default: 30)
#         --output DIR            Output directory (default: <repo>/perf-results/<timestamp>)
#         --flamegraph-dir DIR    Path to an existing FlameGraph clone
#     -i, --interactive           Interactive step-by-step guided mode
#     -h, --help                  Show this help

set -euo pipefail

# Preserve original CLI args (used for sudo re-run hints)
ORIG_ARGS=("$@")

# ---------------------------------------------------------------------------
# Colors (disabled when not on a TTY)
# ---------------------------------------------------------------------------
if [ -t 1 ]; then
    C_RED=$'\033[31m'; C_GREEN=$'\033[32m'; C_YELLOW=$'\033[33m'
    C_BLUE=$'\033[34m'; C_BOLD=$'\033[1m'; C_RESET=$'\033[0m'
else
    C_RED=''; C_GREEN=''; C_YELLOW=''; C_BLUE=''; C_BOLD=''; C_RESET=''
fi

log_info()  { printf '%s[INFO]%s %s\n'    "${C_BLUE}" "${C_RESET}" "$*"; }
log_ok()    { printf '%s[ OK ]%s %s\n'    "${C_GREEN}" "${C_RESET}" "$*"; }
log_warn()  { printf '%s[WARN]%s %s\n'    "${C_YELLOW}" "${C_RESET}" "$*"; }
log_error() { printf '%s[ERROR]%s %s\n'   "${C_RED}" "${C_RESET}" "$*" >&2; }
log_cmd()   { printf '%s  $ %s%s\n'       "${C_BOLD}" "$*" "${C_RESET}"; }

# ---------------------------------------------------------------------------
# Defaults
# ---------------------------------------------------------------------------
DEFAULT_PROGRAM="dde-grand-search-daemon"
PROGRAM="$DEFAULT_PROGRAM"
PID=""
COMMAND=""
FREQ=99
CALL_GRAPH="dwarf"
DURATION=30
OUTPUT_DIR=""
FLAMEGRAPH_DIR=""
INTERACTIVE=0

usage() {
    sed -n '/^# Perf Flame Graph Profiling Script$/,/^#     -h, --help/p' "${BASH_SOURCE[0]}" \
        | sed 's/^# \{0,1\}//'
}

die() { log_error "$*"; exit 1; }

# Ask the user a yes/no question (returns 0 on yes).
# When stdin is not a TTY (e.g. piped/CI), it cannot ask and returns 1.
confirm() {
    local prompt="$1" default="${2:-n}" ans
    [ -t 0 ] || return 1
    if [ "$default" = "y" ]; then
        printf '%s%s%s [Y/n]: ' "${C_BOLD}" "$prompt" "${C_RESET}"
    else
        printf '%s%s%s [y/N]: ' "${C_BOLD}" "$prompt" "${C_RESET}"
    fi
    read -r ans
    ans="${ans:-$default}"
    case "$ans" in
        y|Y|yes|YES) return 0 ;;
        *) return 1 ;;
    esac
}

# ---------------------------------------------------------------------------
# Argument parsing
# ---------------------------------------------------------------------------
while [ $# -gt 0 ]; do
    case "$1" in
        -p|--pid)           PID="${2:-}"; shift 2 ;;
        --program)          PROGRAM="${2:-}"; shift 2 ;;
        --command)          COMMAND="${2:-}"; shift 2 ;;
        --freq)             FREQ="${2:-}"; shift 2 ;;
        --call-graph)       CALL_GRAPH="${2:-}"; shift 2 ;;
        -d|--duration)      DURATION="${2:-}"; shift 2 ;;
        --output)           OUTPUT_DIR="${2:-}"; shift 2 ;;
        --flamegraph-dir)   FLAMEGRAPH_DIR="${2:-}"; shift 2 ;;
        -i|--interactive)   INTERACTIVE=1; shift ;;
        -h|--help)          usage; exit 0 ;;
        *)                  die "未知参数: $1 (使用 --help 查看帮助)" ;;
    esac
done

# ---------------------------------------------------------------------------
# Paths
# ---------------------------------------------------------------------------
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="$(git -C "$SCRIPT_DIR" rev-parse --show-toplevel 2>/dev/null)"
if [ -z "$PROJECT_ROOT" ]; then
    PROJECT_ROOT="$(cd "$SCRIPT_DIR/../.." && pwd)"
fi

# ---------------------------------------------------------------------------
# Dependency detection
# ---------------------------------------------------------------------------
check_perf() {
    if command -v perf >/dev/null 2>&1; then
        log_ok "已检测到 perf: $(command -v perf) ($(perf --version 2>/dev/null | head -n1))"
        return 0
    fi

    log_error "未检测到 perf 工具。"
    cat <<EOF
perf 需要匹配当前内核版本，通常可用发行版包安装：

    sudo apt-get update
    sudo apt-get install linux-perf
    # 若包名不存在，可尝试：
    #   sudo apt-get install linux-tools-common linux-tools-\$(uname -r)
EOF

    if confirm "是否现在尝试安装 perf？(需要 sudo 权限)" "y"; then
        log_info "正在安装 perf ..."
        if sudo -n true 2>/dev/null; then
            sudo apt-get update && sudo apt-get install -y linux-perf
        else
            log_warn "安装需要 sudo 权限，将提示输入密码。"
            sudo apt-get update && sudo apt-get install -y linux-perf
        fi
        if command -v perf >/dev/null 2>&1; then
            log_ok "perf 安装完成: $(command -v perf)"
            return 0
        fi
        log_warn "未能确认 perf 安装成功，请手动安装后重跑。"
    else
        log_info "已跳过安装。"
    fi
    return 1
}

resolve_flamegraph_dir() {
    # 1) explicit --flamegraph-dir
    if [ -n "$FLAMEGRAPH_DIR" ]; then
        if [ -d "$FLAMEGRAPH_DIR" ]; then
            return 0
        fi
        log_warn "--flamegraph-dir 指向的目录不存在: $FLAMEGRAPH_DIR"
    fi

    # 2) default user location
    if [ -d "$HOME/.local/share/FlameGraph" ]; then
        FLAMEGRAPH_DIR="$HOME/.local/share/FlameGraph"
        return 0
    fi

    # 3) on PATH
    if command -v flamegraph.pl >/dev/null 2>&1; then
        FLAMEGRAPH_DIR="$(cd "$(dirname "$(command -v flamegraph.pl)")" && pwd)"
        return 0
    fi

    FLAMEGRAPH_DIR=""
    return 1
}

check_flamegraph() {
    if resolve_flamegraph_dir; then
        local scp="$FLAMEGRAPH_DIR/stackcollapse-perf.pl"
        local fg="$FLAMEGRAPH_DIR/flamegraph.pl"
        if [ -x "$scp" ] && [ -x "$fg" ]; then
            log_ok "已检测到 FlameGraph 工具: $FLAMEGRAPH_DIR"
            return 0
        fi
        log_warn "目录 $FLAMEGRAPH_DIR 存在但缺少 flamegraph.pl / stackcollapse-perf.pl"
    fi

    log_warn "未检测到 FlameGraph 工具（flamegraph.pl / stackcollapse-perf.pl）。"

    local clone_dir="$HOME/.local/share/FlameGraph"
    cat <<EOF
需要 Brendan Gregg 的 FlameGraph 仓库：

    git clone https://github.com/brendangregg/FlameGraph.git ~/.local/share/FlameGraph
EOF

    if confirm "是否现在克隆 FlameGraph 到 ${clone_dir}？" "y"; then
        log_info "正在克隆 FlameGraph ..."
        mkdir -p "$HOME/.local/share"
        if git clone https://github.com/brendangregg/FlameGraph.git "$clone_dir"; then
            FLAMEGRAPH_DIR="$clone_dir"
            log_ok "FlameGraph 克隆完成: $clone_dir"
            return 0
        fi
        log_error "克隆失败，请检查网络或手动克隆后重跑。"
    else
        log_info "已跳过克隆。"
    fi
    return 1
}

# ---------------------------------------------------------------------------
# Permission check
# ---------------------------------------------------------------------------
check_permission() {
    local paranoid
    paranoid="$(cat /proc/sys/kernel/perf_event_paranoid 2>/dev/null || echo unknown)"
    case "$paranoid" in
        -1|0|1)
            log_ok "perf 权限正常 (perf_event_paranoid=$paranoid)"
            ;;
        2)
            log_warn "perf_event_paranoid=2：仅允许用户态 CPU 采样，内核符号将被屏蔽。"
            log_warn "如需分析内核栈，请用 root 重跑：  sudo $0 ${ORIG_ARGS[*]:-}"
            ;;
        3|*)
            log_warn "perf_event_paranoid=$paranoid：用户态采样可能受限。"
            log_warn "建议用 root 重跑，或降低限制： sudo sysctl kernel.perf_event_paranoid=1"
            ;;
    esac
}

# ---------------------------------------------------------------------------
# PID resolution
# ---------------------------------------------------------------------------
find_pid() {
    local prog="$1"
    local pid
    # comm 字段被内核截断为 15 字符，优先匹配完整命令行
    pid="$(pgrep -f "[/ ]${prog}([[:space:]]|$)" 2>/dev/null | head -n1 || true)"
    if [ -z "$pid" ]; then
        pid="$(pgrep -f "${prog}" 2>/dev/null | head -n1 || true)"
    fi
    printf '%s\n' "$pid"
}

resolve_target() {
    if [ -n "$COMMAND" ]; then
        log_info "启动模式：将对以下命令采样"
        log_cmd "$COMMAND"
        return 0
    fi

    if [ -n "$PID" ]; then
        if kill -0 "$PID" 2>/dev/null; then
            log_ok "附加到指定 PID: $PID"
            return 0
        fi
        die "PID $PID 不存在或无法访问。"
    fi

    local pid
    pid="$(find_pid "$PROGRAM")"
    if [ -z "$pid" ]; then
        log_error "未找到运行中的进程: $PROGRAM"
        cat <<EOF
请确认目标程序已启动，或改用以下方式之一：
    $0 --pid <PID>                     # 直接指定 PID
    $0 --program <进程名>               # 指定其他程序名
    $0 --command "<可执行文件> [参数]"  # 由 perf 拉起进程采样
EOF
        return 1
    fi
    PID="$pid"
    log_ok "找到进程 $PROGRAM -> PID $PID"
    return 0
}

# ---------------------------------------------------------------------------
# Interactive mode
# ---------------------------------------------------------------------------
interactive_prompt() {
    local prompt="$1" default="$2" ans
    printf '%s%s%s [默认: %s]: ' "${C_BOLD}" "$prompt" "${C_RESET}" "$default"
    read -r ans
    printf '%s\n' "${ans:-$default}"
}

run_interactive() {
    echo
    log_info "===== 交互式引导 ====="

    PROGRAM="$(interactive_prompt "要分析的程序名" "$PROGRAM")"

    local freq_ans
    freq_ans="$(interactive_prompt "采样频率(Hz)" "$FREQ")"
    FREQ="$freq_ans"

    local cg_ans
    cg_ans="$(interactive_prompt "调用栈展开方式 (dwarf/fp)" "$CALL_GRAPH")"
    CALL_GRAPH="$cg_ans"

    local dur_ans
    dur_ans="$(interactive_prompt "采样时长(秒，0=手动 Ctrl+C)" "$DURATION")"
    DURATION="$dur_ans"
}

# ---------------------------------------------------------------------------
# .gitignore
# ---------------------------------------------------------------------------
update_gitignore() {
    local gitignore="$PROJECT_ROOT/.gitignore"
    local entry="perf-results/"
    if [ ! -f "$gitignore" ]; then
        printf '%s\n' "$entry" > "$gitignore"
        log_ok "已创建 .gitignore 并忽略 perf-results/"
    elif ! grep -qF "$entry" "$gitignore"; then
        printf '%s\n' "$entry" >> "$gitignore"
        log_ok "已在 .gitignore 中追加 perf-results/"
    else
        log_info ".gitignore 已包含 perf-results/，跳过"
    fi
}

# ---------------------------------------------------------------------------
# Main flow
# ---------------------------------------------------------------------------
main() {
    echo
    log_info "===== Perf 火焰图性能分析 ====="

    if [ "$INTERACTIVE" = 1 ]; then
        run_interactive
    fi

    # Dependencies
    check_perf || exit 1
    check_flamegraph || exit 1
    check_permission

    # Target
    resolve_target || exit 1

    # Output dir
    if [ -z "$OUTPUT_DIR" ]; then
        OUTPUT_DIR="$PROJECT_ROOT/perf-results/$(date +%Y%m%d-%H%M%S)"
    fi
    mkdir -p "$OUTPUT_DIR"
    log_ok "输出目录: $OUTPUT_DIR"

    # .gitignore
    update_gitignore

    local perf_data="$OUTPUT_DIR/perf.data"
    local perf_script="$OUTPUT_DIR/perf.script.txt"
    local folded="$OUTPUT_DIR/out.folded"
    local report="$OUTPUT_DIR/perf.report.txt"
    local svg="$OUTPUT_DIR/flamegraph.svg"

    # --- Record ---
    local -a record_args=( record -F "$FREQ" -g --call-graph "$CALL_GRAPH" -o "$perf_data" )

    echo
    log_info "开始采样 (${C_BOLD}${FREQ}${C_RESET}Hz, call-graph=${C_BOLD}${CALL_GRAPH}${C_RESET}) ..."

    if [ -n "$COMMAND" ]; then
        # Launch mode
        if [ "$DURATION" -gt 0 ] 2>/dev/null; then
            log_info "将在 ${DURATION}s 后自动停止"
            timeout --signal=INT "$DURATION" perf "${record_args[@]}" -- $COMMAND || true
        else
            log_info "按 Ctrl+C 停止采样"
            perf "${record_args[@]}" -- $COMMAND
        fi
    else
        # Attach mode
        record_args+=( -p "$PID" )
        if [ "$DURATION" -gt 0 ] 2>/dev/null; then
            log_info "将在 ${DURATION}s 后自动停止"
            perf "${record_args[@]}" -- sleep "$DURATION"
        else
            log_info "按 Ctrl+C 停止采样"
            perf "${record_args[@]}"
        fi
    fi

    if [ ! -s "$perf_data" ]; then
        log_error "未生成 perf.data，采样可能失败（若提示权限不足，请用 sudo 重跑）。"
        exit 1
    fi
    log_ok "采样完成: $perf_data"

    # --- Report / Flame graph ---
    log_info "生成文本报告 ..."
    perf report -i "$perf_data" --stdio > "$report" 2>/dev/null || \
        log_warn "perf report 失败（不影响火焰图生成）"

    log_info "展开调用栈 ..."
    perf script -i "$perf_data" > "$perf_script"
    "$FLAMEGRAPH_DIR/stackcollapse-perf.pl" "$perf_script" > "$folded"

    log_info "生成火焰图 ..."
    "$FLAMEGRAPH_DIR/flamegraph.pl" "$folded" > "$svg"

    echo
    log_ok "===== 完成 ====="
    log_info "火焰图:   $svg"
    log_info "折叠报告: $folded"
    log_info "文本报告: $report"
    log_info "原始数据: $perf_data"
    echo
    log_info "用浏览器打开火焰图查看（可交互缩放）:"
    log_cmd "xdg-open \"$svg\""
    echo
}

main "$@"
