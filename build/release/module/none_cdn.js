/**
 * 检测非CDN链路请求（SINA CDN 专用）
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
  selector.nonecdn();
  
  var noneCDNUrls = selector.get();

  if (!noneCDNUrls || noneCDNUrls.length == 0) {
    return "所有请求均为CDN链路。";
  }
  
  var message = supplant("如下 URL 为非 CDN 链路, 共 ${count} 个: \n", {
    count: noneCDNUrls.length
  });
  
  for (var i = 0, c = noneCDNUrls.length; i < c; ++i) {
    message += supplant("${url}\n", {
      url: noneCDNUrls[i].url
    });
  }

  return message;
});