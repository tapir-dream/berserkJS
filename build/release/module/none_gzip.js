/**
 * 检测没有未gzip的url
 * 通过 App.selector 方法获取数据 不需要传入 networkData
 * @return {string} 返回报告内容
 * @author Tapir | baokun@staff.sina.com.cn
 * @date   2012-03-08
 */
(function () {

  var supplant = App.helper.supplant;
  var selector = App.selector;
  // 清除老选择内容
  selector.clear();
  // 开始新的选择
  selector.nonegzip();
  
  var noneGZipUrls = selector.get();

  if (!noneGZipUrls || noneGZipUrls.length == 0) {
    return "所有请求均启用了GZip。";
  }
  
  var message = supplant("如下 URL 没有被GZip, 共 ${count} 个,建议启用: \n", {
    count: noneGZipUrls.length
  });
  
  for (var i = 0, c = noneGZipUrls.length; i < c; ++i) {
    message += supplant("url: ${url}; size: ${size} kb \n", {
      url: noneGZipUrls[i].url,
      size: (noneGZipUrls[i].ResponseSize / 1024).toFixed(2)
    });
  }

  return message;
});