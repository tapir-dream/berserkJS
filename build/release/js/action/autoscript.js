(function (runTaskModuleHandle) {
  var w = App.webview; 
  // 取数据
  var getNetworkData = function() {
    w.removeEventListener("load", getNetworkData);
    // 避免过早关闭监听 因为微博是BigPipe
    // 当页面一段时间没有变化后再处理
    // 这会导致weibo心跳认为连接失效
    // 将用户踢出登录状态
    var id = 0;
    var interval = 600;
    var repaint = function() {
      w.clearTimeout(id);
      id = w.setTimeout(function() {
        runTaskModuleHandle();
        w.removeEventListener("repaint", repaint);
        w.netListener(false);
      }, interval);
    };
    w.addEventListener("repaint", repaint);
  };

  // 实现微博登录；
  var loginWeibo = function() {
    w.removeEventListener("load", loginWeibo);
    w.addEventListener("load", getNetworkData);
    w.execScript(function() {
      // 输入测试账号密码
      //document.getElementById("loginname").value = "";
      //document.getElementById("password").value = "";
      alert('action\autoscript.js line 30, 需要用你的账号与密码哦~~亲~~');
    });
    // 点下登录按键
    w.sendMouseEvent(w.elementRects("#login_submit_btn")[0]);
    // 开启网络监听
    w.netListener(true);
  };

  // 加载工具函数库
  App.loadScript(App.path + "helper.js", function(err, func) {
    if (err)
      return;
    func();
  });

  w.addEventListener("load", loginWeibo);
  w.open("http://www.weibo.com");
});
