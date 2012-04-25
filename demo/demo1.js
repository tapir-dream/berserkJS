// 开启监听
App.netListener(true);
// 访问页面
App.webview.open("http://www.weibo.com"); 
setTimeout(function() {
  // 获取数据并序列化
  var data = JSON.stringify(App.networkData()); 
  // 写入文件
  App.writeFile(App.path + "demo1.txt", data);
  // 关闭监听
  App.netListener(false); 
  // 退出应用
  App.close(); 
}, 3000);

