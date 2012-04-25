// 访问页面
App.webview.open("http://www.weibo.com"); 
// 监听文档加载完成
App.webview.addEventListener("load", function() {
  // 进入页面沙箱执行页面脚本
  App.webview.execScript(function() {
    // 输入测试账号密码. 
    //请使用你自己的账号密码
    //document.getElementById("loginname").value = "";
    //document.getElementById("password").value = "";
  });
  // 点下登录按键
  App.webview.sendMouseEvent(w.elementRects("#login_submit_btn")[0]);
});
