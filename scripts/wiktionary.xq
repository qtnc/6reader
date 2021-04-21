(: name: Chercher dans le wiktionnaire :)
(: description: Effectue une recherche dans le wiktionnaire sur fr.wiktionary.org :)
(: icon: dictionary.xpm :)
(: input: prompt :)
(: flags: double-utf8 :)

declare variable $arg external;
declare variable $url := concat("https://Fr.wiktionary.org/wiki/", $arg);
declare variable $doc := xqilla:parse-html(unparsed-text($url, "ISO-8859-1"));
declare variable $outpart := $doc//div[@class="mw-parser-output"];
declare variable $ols := $outpart/ol[count(preceding-sibling::h2)=1];

<html><head>
<meta charset="utf-8"/>
<title>{$arg}</title>
</head><body>
<h1>{$arg}</h1>
<ol>{
for $item in $ols/li
return copy $li := $item 
modify (
for $a in $li//a return replace node $a with data($a)
) 
return $li
}</ol>
<p><a href="{$url}">ARticle complet</a></p>
</body></html>