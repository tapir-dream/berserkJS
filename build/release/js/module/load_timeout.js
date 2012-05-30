/**
 * 检测加载时间超时模块
 * @param {Array} data 指定 network 数据数组
 * @param {number} max 指定超时时间 ms
 * @return {string} 返回报告内容
 * @author Tapir | baokun@staff.sina.com.cn
 * @date   2012-03-06
 */
(function (data, max) {
  var supplant = App.helper.supplant;
  
  var duration = {};
  for (var i = 0, c = data.length; i < c; ++i) {
    if (data[i].ResponseDuration > max) {
      duration[data[i].url] = data[i].ResponseDuration;
    }
  }
  
  var urls = Object.keys(duration);
  var count = urls.length;
  var message = "如下 URL 加载时间大于 ${max} ms: \n";
  for (var i = 0; i < count; ++i) {
    message += supplant("URL: ${url}, Duration: ${time} ms \n", {
      url: urls[i],
      time: duration[urls[i]]
    });
  }
  message = supplant(message, {max: max});
  return message;
});