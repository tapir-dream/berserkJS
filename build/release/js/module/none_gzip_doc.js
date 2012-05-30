/**
 * 检测没有未gzip并且非 IMG 的文档类 url
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
  
  var targetUrls = [];
  for (var i = 0, c = noneGZipUrls.length; i < c; ++i) {
    var noneGZipUrl =  noneGZipUrls[i];
    if (noneGZipUrl.isCssFile || noneGZipUrl.isDocFile || noneGZipUrl.isJsFile) {
      targetUrls.push(noneGZipUrl);
    }
  }

  if (!targetUrls || targetUrls.length == 0) {
    return "所有JS/CSS/HTML请求均启用了GZip。";
  }
  
  var message = supplant("如下 JS/CSS/HTML URL 没有被GZip, 共 ${count} 个,建议启用: \n", {
    count: targetUrls.length
  });
  
  for (var i = 0, c = targetUrls.length; i < c; ++i) {
    message += supplant("url: ${url}; size: ${size} kb \n", {
      url: targetUrls[i].url,
      size: (targetUrls[i].ResponseSize / 1024).toFixed(2)
    });
  }
  return message;
});
