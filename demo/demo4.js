// 访问页面
App.webview.open("http://weibo.com"); 
// 监听文档加载完成
App.webview.addEventListener("load", function(rect) {
  // 截取全页保存为图片
  App.webview.saveImage(App.path + 'demo4.png', 'png', 100);
  App.close();
});
