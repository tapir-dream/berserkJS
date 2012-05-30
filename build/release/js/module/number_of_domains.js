/**
 * 检测所用域名数量
 * @param {Array} data 指定 network 数据数组
 * @param {number} max 指定推荐域名数量
 * @return {string} 返回报告内容
 * @author Tapir | baokun@staff.sina.com.cn
 * @date   2012-03-06
 */
(function (data, max) {
  var supplant = App.helper.supplant;
  
  var domainRegExp = /^http:\/\/.+?\//;
  var domainMap = {};
  for (var i = 0, c = data.length; i < c; ++i) {
    domainMap[domainRegExp.exec(data[i].url)[0]] = true;
  }
  
  var domains = Object.keys(domainMap);
  var domainCount = domains.length;
  var messageData = {
      domains: domains.join('\n'),
      count: domainCount,
      max: max
  };

  return domainCount > max 
    ? supplant("现使用如下域名：\n ${domains} \n 共 ${count} 个, 请精简域名到 ${max} 个。 ", messageData)
    : supplant("现使用如下域名：\n ${domains} 共 ${count} 个, 符合建议域名总数 ${max} 个范围内。 ", messageData);
});