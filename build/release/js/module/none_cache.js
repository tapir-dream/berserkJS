/**
 * 检测非Cache链路请求
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
  selector.nonecache();
  
  var noneCacheUrls = selector.get();

  if (!noneCacheUrls || noneCacheUrls.length == 0) {
    return "所有请求均为CDN链路。";
  }
  
  var message = supplant("如下 URL 没有被缓存, 共 ${count} 个: \n", {
    count: noneCacheUrls.length
  });
  
  for (var i = 0, c = noneCacheUrls.length; i < c; ++i) {
    message += supplant("${url}\n", {
      url: noneCacheUrls[i].url
    });
  }

  return message;
});