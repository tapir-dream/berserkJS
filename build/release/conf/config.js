/**
 * 配置设置模块
 * @return {object} 返回配置对象
 * @author Tapir | baokun@staff.sina.com.cn
 * @date   2012-03-07               
 */
(function() {

  var path = App.path;
  
  var namespace = function(name) {
    return path + name.replace(/\./g, '/') + '.js';
  };
  
  return {
    // 全局依赖模块表
    // 表内模块被最先被执行
    // 依赖关系由上至下
    global: [
      namespace('action.helper')
    ], 
    
    // 自动化脚本位置，如果为null则直接执行module部分
    // 如果有内容则会在自动化脚本内确切时间点执行module代码
    automation: namespace('action.autoscript'),

    // 运行模块配置表
    // 依赖关系由上至下
    // args 为模块依赖参数。
    // 如果参数是运行时动态指定的，
    // 可以写为function，return array
    // 它将在模块被调用时执行
    module: [
      { 
        path: namespace('module.none_expires'),
        args: function () {
          return [App.networkData()];
        }
      },
      { 
        path: namespace('module.none_gzip_doc'),
        args: []
      },
      { 
        path: namespace('module.none_gzip'),
        args: []
      },
      { 
        path: namespace('module.use_cookie'),
        args: []
      },
      { 
        path: namespace('module.304_cache'),
        args: []
      },
      { 
        path: namespace('module.cookie_from_cdn'),
        args: []
      },
      { 
        path: namespace('module.load_timeout'),
        args: function () {
          return [App.networkData(), 200];
        }
      },
      { 
        path: namespace('module.none_cache'),
        args: []
      },   
            { 
        path: namespace('module.none_cdn'),
        args: []
      }, 
      { 
        path: namespace('module.number_of_domains'),
        args: function () {
          return [App.networkData(), 12];
        }
      }, 
      { 
        path: namespace('module.redirect'),
        args: []
      }
    ],
    
    // 完成时操作列表
    // 将在所有模块运行完成后执行
    completed: [
      namespace('action.report')
    ]
  };
});