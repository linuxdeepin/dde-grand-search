/*
 * Copyright (C) 2021 Uniontech Software Technology Co., Ltd.
 *
 * Author:     liuzhangjian<liuzhangjian@uniontech.com>
 *
 * Maintainer: liuzhangjian<liuzhangjian@uniontech.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
#include "audio-preview/audiofileinfo.h"

#include <stubext.h>

#include <gtest/gtest.h>
#include <fileref.h>
#include <taglib.h>

class TestTag : public TagLib::Tag
{
public:
    TestTag() : TagLib::Tag() {}
    TagLib::String title() const override
    {
        return TagLib::String("title");
    }
    TagLib::String artist() const override
    {
        return TagLib::String("artist");
    }
    TagLib::String album() const override
    {
        return TagLib::String("album");
    }
    TagLib::String comment() const override
    {
        return TagLib::String("comment");
    }
    TagLib::String genre() const override
    {
        return TagLib::String("genre");
    }
    unsigned int year() const override
    {
        return 0;
    }
    unsigned int track() const override
    {
        return 0;
    }
    void setTitle(const TagLib::String &s) override { Q_UNUSED(s) }
    void setArtist(const TagLib::String &s) override { Q_UNUSED(s) }
    void setAlbum(const TagLib::String &s) override { Q_UNUSED(s) }
    void setComment(const TagLib::String &s) override { Q_UNUSED(s) }
    void setGenre(const TagLib::String &s) override { Q_UNUSED(s) }
    void setYear(unsigned int i) override { Q_UNUSED(i) }
    void setTrack(unsigned int i) override { Q_UNUSED(i) }
};

TEST(AudioFileInfoTest, ut_openAudioFile)
{
    stub_ext::StubExt st;
    st.set_lamda(&TagLib::FileRef::tag, [](){ return nullptr; });
    st.set_lamda(&TagLib::FileRef::file, [](){ return nullptr; });

    AudioFileInfo info;
    EXPECT_NO_FATAL_FAILURE(info.openAudioFile("/test.mp3"));

    st.reset(&TagLib::FileRef::file);
    EXPECT_NO_FATAL_FAILURE(info.openAudioFile("/test.mp3"));
}

TEST(AudioFileInfoTest, ut_characterEncodingTransform)
{
    TestTag tag;
    AudioFileInfo::AudioMetaData meta;
    AudioFileInfo info;
    info.characterEncodingTransform(meta, &tag);

    EXPECT_TRUE(meta.artist == "artist");
    EXPECT_TRUE(meta.album == "album");
}
