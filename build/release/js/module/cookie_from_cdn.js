/**
 * 检测CDN链路请求并且带有Cookie 头的模块（SINA CDN 专用）
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
  selector.fromcdn();
  selector.cookie();
  
  var hasCookieUrls = selector.get();

  if (!hasCookieUrls || hasCookieUrls.length == 0) {
    return "所有CDN链路请求均已经优化 Cookie。";
  }
  
  var message = supplant("如下 URL 为 CDN 链路并带有 Cookie 头, 共 ${count} 个: \n", {
    count: hasCookieUrls.length
  });
  
  for (var i = 0, c = hasCookieUrls.length; i < c; ++i) {
    message += supplant("${url}\n", {
      url: hasCookieUrls[i].url
    });
  }

  return message;
});