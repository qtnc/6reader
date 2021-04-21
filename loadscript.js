(function(){

function findElement (indices) {
var e = document.getElementsByTagName('html')[0].parentNode;
indices = indices.split(',') .map(function(x){ return parseInt(x.trim(), 10); });
for (var i=0, n=indices.length; i<n && e; i++) e = e.childNodes[indices[i]];
return e;
}

function getElementPath (e) {
var l = [];
while(e&&e.parentNode){
var index, children = e.parentNode.childNodes;
for (var i=0, n=children.length; i<n; i++) if (children[i]==e) { index=i; break; }
l.push(index);
e = e.parentNode;
}
l.reverse();
return l.join(',');
}

function isFocusable (e) {
var tag = e.tagName.toLowerCase(), tabindex = e.getAttribute('tabindex') || -10;
return tabindex>=-1 || ['a', 'input', 'textarea', 'button'].indexOf(tag)>=0;
}

function moveFocus (e) {
if (!e) return false;
var focusable = isFocusable(e);
if (!focusable) e.setAttribute('tabindex', -1);
if (Document.activeElement==e) e.blur();
e.focus();
return true;
}

window.___epubJumpToPosition = function (strPos) {
var sel = window.getSelection(), t = strPos.split(';');
if (!sel || t.length!=4) return false;
var startNode = findElement(t[0]), endNode = findElement(t[2]), focusNode = startNode, startPos = parseInt(t[1], 10) || 0, endPos = parseInt(t[3], 10) || 0;
if (!startNode || !endNode || !focusNode) return false;
if (focusNode.nodeType==3) focusNode = focusNode.parentNode;
var result = moveFocus(focusNode);
var range = (document.createRange && document.createRange()) || new Range();
range.setStart(startNode, startPos);
range.setEnd(endNode, endPos);
sel.removeAllRanges();
sel.addRange(range);
return result;
};

window.___epubGetCurrentPosition = function () {
var sel = window.getSelection();
if (!sel) return "";
var startNode = sel.anchorNode, startPos = sel.anchorOffset || 0, endNode = sel.focusNode, endPos = sel.focusOffset || 0;
if (!startNode || !endNode) return "";
var startPath = getElementPath(startNode), endPath = getElementPath(endNode);
return startPath + ';' + startPos + ';' + endPath + ';' + endPos;
};

document.body.addEventListener('mouseup', function(e){
if (e.button==2) {
document.title = "internal:context-menu";
}
});

/*document.body.addEventListener('contextmenu', function(e){
e.preventDefault();
var sel = window.getSelection();
    var range = sel.getRangeAt(0);
    var node = sel.anchorNode;
    while (range.toString().indexOf(' ') != 0 && range.startOffset>0) {
        range.setStart(node, (range.startOffset - 1));
    }
if (range.startOffset>0)     range.setStart(node, range.startOffset + 1);
    do {
        range.setEnd(node, range.endOffset + 1);
    } while (range.toString().indexOf(' ') == -1 && range.toString().trim() != '' && range.endOffset < node.length);
    var word = range.toString().trim();
alert(word + ', ' + e.buttons);
});*/

})();
//alert('test');