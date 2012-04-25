/**
 * 检测没有 Expires 头的 URL
 * @param {Array} data 指定 network 数据数组
 * @return {string} 返回报告内容
 * @author Tapir | baokun@staff.sina.com.cn
 * @date   2012-03-08
 */
(function (data) {

  var supplant = App.helper.supplant;
 
  var targetUrls = [];
  for (var i = 0, c = data.length; i < c; ++i) {
    if (data[i].Expires.trim() == '') {
      targetUrls.push(data[i]);
    }
  }

  if (!targetUrls || targetUrls.length == 0) {
    return "所有请求均设置了 Expires 头。";
  }
  
  var message = supplant("如下 URL 没有设置 Expires 头, 共 ${count} 个,建议设置: \n", {
    count: targetUrls.length
  });
  
  for (var i = 0, c = targetUrls.length; i < c; ++i) {
    message += supplant("url: ${url}\n", {
      url: targetUrls[i].url,
    });
  }
  return message;
});
