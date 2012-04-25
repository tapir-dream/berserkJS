// 访问页面
App.webview.open("http://weibo.com/login.php"); 
// 渲染计数
var repaintCount = 0;
// 监听文档渲染
App.webview.addEventListener("repaint", function(rect) {
  rect.repaintCount = repaintCount++;
  // 进入页面沙箱执行页面脚本
  App.webview.execScript(function(rect) {
    // 控制台输出，页面输出会触发 repaint，然后…… 你懂的……
    console.log( "repaint count:" + rect.repaintCount,
      "repaint rect: ",
      "x: " + rect.x,
      "y: " + rect.y,
      "width: " + rect.width,
      "height: " + rect.height
      );
  }, rect);
});
