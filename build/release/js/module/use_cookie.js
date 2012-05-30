/**
 * 检测携带cookie头的请求
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
  selector.cookie();
  
  var hasCookieUrls = selector.get();

  if (!hasCookieUrls || hasCookieUrls.length == 0) {
    return "所有请求均没有携带Cookie头。";
  }
  
  var message = "如下 URL 带有 Cookie 头: \n";
  for (var i = 0, c = hasCookieUrls.length; i < c; ++i) {
    message += supplant("${url}\n", {
      url: hasCookieUrls[i].url
    });
  }

  return message;
});