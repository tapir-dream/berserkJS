// 打开目标页
App.webview.open('http://www.weibo.com');
// 监听首次渲染事件
App.webview.addEventListener('firstPaintFinished', function(timeout, url) {
  this.execScript(function (o) {
    var node = document.createElement('div');
    var cssText = 'font-size: 14px; background:yellow; border: 1px solid #000;';
    cssText += 'position: fixed; top: 0; left:0; z-index:99999;';
    node.style.cssText = cssText;
    node.innerHTML = '<ul><li>' + o.timeout + ' ms</li>' +
      '<li> ' + o.url + '</li></ul>';
    document.body.appendChild(node);
  }, {timeout: timeout, url: url});
});
