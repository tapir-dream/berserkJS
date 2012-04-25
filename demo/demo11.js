// 输出命令行参数
console.log('所有命令行参数是：' + App.args)

// 检测指定参数内容
if (App.args[1] != 'weibo') {
  App.close();
}

// 加载脚本文件
App.loadScript(App.path + 'conf\\init.js', function(err, func) {
  // 检查脚本是否可用
  if (err) {
    alert('Hi~ Load Script Error!')
  }
  // 执行脚本
  func(App, App.webview);
});
