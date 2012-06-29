// 访问页面
App.webview.open('http://weibo.com/login.php'); 
// 监听文档加载完成
App.webview.addEventListener('load', function(rect) {
  // 截取登录表单区域保存为图片
  App.webview.saveImage(App.path + 'demo5.png', 'png', 100, 
    App.webview.elementRects('#login_form')[0]);
  App.close();
});
