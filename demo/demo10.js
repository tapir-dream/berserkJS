// 自定义首屏时间需校准的检测区域
App.webview.setDetectionRects([
  {x:10, y:40, width:100, height:100}, 
  {x:500, y:400, width:80, height:80}
] , 0.8);
// 打开目标页
App.webview.open('http://www.163.com');
// 监听首屏渲染事件
App.webview.addEventListener('firstScreenFinished', function(timeout, url) {
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
