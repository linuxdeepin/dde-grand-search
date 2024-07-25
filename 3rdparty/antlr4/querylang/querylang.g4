grammar querylang;
query: primary | invalideQuery ;

invalideQuery: 'ERROR' string;

primary:
    binaryExpression ('AND' binaryExpression | 'OR' binaryExpression)*
    ;

binaryExpression:
    '(' primary ')'
    | searchCondition
    ;


searchCondition:
	dateSearch
	| pathSearch
	| nameSearch
	| sizeSearch
	| typeSearch
	| durationSearch
	| metaSearch
	| quantityCondition
	| contentSearch;


dateSearch: 
    binaryDateSearch ('AND' binaryDateSearch | 'OR' binaryDateSearch)*
    ;

binaryDateSearch:
    '(' dateSearch ')'
    | dateSearchinfo
    ;

dateSearchinfo: 
    'DATE' comparison_type absolutedate
    | 'DATE' comparison_type relativelydate
    ;

absolutedate: string;
relativelydate: 'CURRENT' '-' string;

pathSearch: 'PATH' is_or_not string;
nameSearch: 'NAME CONTAINS' string;
sizeSearch:
	'SIZE' comparison_type string
	| 'SIZE' comparison_type 'FILE_SIZE' filename ;
typeSearch: 'TYPE' is_or_not string;
durationSearch: 'DURATION' comparison_type string;
metaSearch:
	'META_TYPE' 'IS' string 'AND' 'META_VALUE' is_or_not string;
quantityCondition: 'QUANTITY' '=' NUMBER_VALUE;
contentSearch:
	'CONTENT CONTAINS' string
	| 'CONTENT EQUALS FILE' filename;

comparison_type: '=' | '<' | '>' | '!=' | '<=' | '>=';

filename: string;
string: STRING_VALUE;
STRING_VALUE: '"' .*? '"';

is_or_not: 'IS' | 'IS NOT';
NUMBER_VALUE: [0-9]+;

WS : [ \t\n\r]+ -> skip ;