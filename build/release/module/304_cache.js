/**
 * 检测 304 缓存协商
 * 通过 App.selector 方法获取数据 不需要传入 networkData
 * @return {string} 返回报告内容
 * @author Tapir | baokun@staff.sina.com.cn
 * @date   2012-03-06
 */
(function () {

  var supplant = App.helper.supplant;
  var selector = App.selector;
  // 清除老选择内容
  selector.clear();
  // 开始新的选择
  selector.http304();
  
  var http304Urls = selector.get();

  var message = ""
  
  if (!http304Urls || http304Urls.length == 0) {
    return "无 304 缓存协商";
  }


  message += supplant("如下 URL 存在 304 重定向, 共 ${count} 个: \n", {
    count: http304Urls.length
  });
  
  for (var i = 0, c = http302Urls.length; i < c; ++i) {
    message += supplant("url: ${url}\n, time: ${duration} ms", {
      url: http302Urls[i].url,
      duration: http302Urls[i].ResponseDuration
    });
  }
  
  return message;
});