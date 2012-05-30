/**
 * 常用工具方法模块
 *
 * @author Tapir | baokun@staff.sina.com.cn
 * @date   2012-03-06
 */
(function() {

  var supplant = function(str, data, regexp) {
    return str.replace( regexp || /\${([^{}]*)}/g,
      function(str, p1) {
        return data[p1].toString();
      });
  };

  var require = function(moduleName, args) {
    if (Array.isArray(args))
      args = [];
    var module = App.loadScript(App.path + 'module/' + moduleName + '.js',
      function(err, module) {
        if (err)
          return false;
        return module.apply(null, args);
      });
    
    return (!module) ? null : module;
  };
  
  var alert = function(msg) {
    web.execScript(function(msg) {
      alert(msg);
    }, msg);
  }
  
  var log = function(msg) {
    web.execScript(function(msg) {
      console.log(msg);
    }, msg);
  } 
  
  App.helper = {
    supplant: supplant,
    require: require,
    alert: alert,
    log: log
  };
  
  return App.helper;
});