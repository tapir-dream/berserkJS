/**
 * 检测处理完成
 *
 * @author Tapir | baokun@staff.sina.com.cn
 * @date   2012-03-16
 */
(function(msg) {
  app.writeFile(App.path + 'test/log.txt', msg.join('\n'));
  
  console.log('finished!!!');
  console.log('quit after 3 seconds~~~');
  
  App.setTimeout(function() {
    App.close();
  }, 3000);

});