(: name: Chercher sur Wikipédia :)
(: description: Effectue une recherche sur wikipédia en français :)
(: icon: wikipedia.xpm :)
(: input: prompt :)
(: flags: double-utf8 :)

declare variable $arg external;
declare variable $url := concat("http://fr.wikipedia.org/wiki/", $arg);
declare variable $doc := xqilla:parse-html(unparsed-text($url, "ISO-8859-1"));
declare variable $outpart := $doc//div[@class="mw-parser-output"];

<html>
<head>
<meta charset="utf-8"/>
{$doc/title}
</head>
<body>
<h1>{$arg}</h1>
{
for $para in $outpart/*
where (name($para)="p" or name($para)="ul" or name($para)="ol")
and $para/following-sibling::div[@id="toc"]
return copy $p := $para
modify (
delete nodes $p//sup,
for $a in $p//a return replace node $a with data($a)	
)
return $p
}
<p><a href="{$url}">Article complet</a></p>
</body></html>
