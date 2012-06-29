// 访问页面
App.webview.open('http://www.weibo.com'); 
// 监听文档加载完成
App.webview.addEventListener('load', function() {
  // 进入页面沙箱执行页面脚本
  App.webview.execScript(function() {
    // 输入测试账号密码, 请自行添加用户名与密码
    //document.querySelectorAll('input[node-type=loginname]')[0].value = 'xxxxx';
    //document.querySelectorAll('input[node-type=password]')[0].value = 'xxxxx';
  });
  // 点下登录按键
  App.webview.sendMouseEvent(App.webview.elementRects('a[node-type=submit]')[0]);
});

