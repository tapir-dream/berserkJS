/**
 * 检测 301 302 重定向的请求
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
  selector.http301();
  
  var http301Urls = selector.get();

  var message = ""
  
  if (!http301Urls || http301Urls.length == 0) {
    message += "无 301 重定向\n";
  } else {
    message += supplant("如下 URL 被 301 重定向, 共 ${count} 个: \n", {
      count: http301Urls.length
    });
    
    for (var i = 0, c = http301Urls.length; i < c; ++i) {
      message += supplant("${url}\n", {
        url: http301Urls[i].url
      });
    }
  }
  
  
  // 清除老选择内容
  selector.clear();
  // 开始新的选择
  selector.http302();

  var http302Urls = selector.get();

  if (!http302Urls || http302Urls.length == 0) {
    message += "无 302 重定向";
  } else {
    message += supplant("如下 URL 被 302 重定向, 共 ${count} 个: \n", {
      count: http302Urls.length
    });
    
    for (var i = 0, c = http302Urls.length; i < c; ++i) {
      message += supplant("${url}\n", {
        url: http302Urls[i].url
      });
    }  
  }
  

  return message;
});