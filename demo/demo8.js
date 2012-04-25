// 打开目标页
App.webview.open('http://www.baidu.com')
// 监听页面加载完成
App.webview.addEventListener('load', function() {
  // 跨域同步取得请求结果
  var html = App.httpRequest('get', 'http://weibo.com/');
  // 将结果交给浏览器 JS 处理
  this.execScript(function(html) {
    document.body.innerHTML = html;
  }, html);
  // 看下当前 URL
  alert(this.getUrl());
});
