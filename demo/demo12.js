var msg = [];
var oldMem = App.memory();
// 页面脚本刚刚被创建时候检测
App.webview.addEventListener('pageScriptCreated', function() {
  msg[0] = App.cpu() + '%';
  msg[1] = App.memory() - oldMem + 'KB';
});
// 页面初始布局完成后检测
App.webview.addEventListener('initLayoutCompleted', function() {
  msg[2]= App.cpu() + '%';
  msg[3] = App.memory() - oldMem + 'KB';
});
// 页面加载完成后检测
App.webview.addEventListener('load', function() {
  msg[4] = App.cpu() + '%';
  msg[5] = App.memory() - oldMem + 'KB';
  alert(['CPU占用分别是:', msg[0], msg[2], msg[4]].join() + '\n' +
        ['内存占用分别是:', msg[1], msg[3], msg[5]].join());
});
App.webview.open('http://www.weibo.com');
