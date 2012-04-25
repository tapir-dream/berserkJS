/**
 * 运行初始化模块
 * @author Tapir | baokun@staff.sina.com.cn
 * @date   2012-03-07
 */

(function (app, web) {

  var CONFIG_PATH = app.path + 'conf/config.js';
  var CONFIG_LOAD_ERROR_MSG = '配置文件载入错误！';
  var CONFIG_PARAM_ERROR_MSG = '配置文件参数错误！'
  var GLOBAL_MODULE_LOAD_ERROR_MSG = '全局模块载入错误！模块路径为：';

  var config = app.loadScript(CONFIG_PATH, function(err, conf) {
    if (err)
      return false;
    return conf();
  });

  // 检验配置文件是加载成功
  if (config == false) {
    alert(CONFIG_LOAD_ERROR_MSG);
    return;
 }

  // 初始化全局依赖模块
  var initGlobalModule = function (gModuleList) {
    if (!Array.isArray(gModuleList)) {
      alert(CONFIG_PARAM_ERROR_MSG + '[global]');
      return;
    }
    for (var i = 0, c = gModuleList.length; i < c; ++i) {
      var msg = app.loadScript(gModuleList[i], function(err, gModule) {
         if (err)
           return false;
         return gModule();
      });
      if (msg == false) {
        alert(GLOBAL_MODULE_LOAD_ERROR_MSG + gModuleList[i]);
        return;
      }
    }
  };

 
  // 执行所有模块运行
  var runTaskModule = function(taskModuleList) {
    if (!Array.isArray(taskModuleList)) {
      alert(CONFIG_PARAM_ERROR_MSG + '[module]');
      return;
    }
    
    var taskMessage = [];
    for (var i = 0, c = taskModuleList.length; i < c; ++i) {
      var taskModule = taskModuleList[i];
      var path = taskModule.path;
      var args = taskModule.args;

      // 用于检测 args 是否需要运行时初始化。
      // 如果是 array 则直接调用, 为 function 则执行后得到所需参数数组
      if (typeof args != 'function' && !Array.isArray(args))
        args = [];
      if (typeof args == 'function')
        args = args();
      if (!Array.isArray(args))
        args = [];
      
      // 加载并执行具体任务模块
      var msg = app.loadScript(path, function(err, tModule) {
         if (err)
           return false;
         return tModule.apply(null, args);
      });
      
      if (msg == false) {
        alert(CONFIG_LOAD_ERROR_MSG + path);
        return;
      }
      // 将任务模块返回值逐一加入数组
      taskMessage.push(msg);
    }
    return taskMessage;
  };
  
  // 装配全部模块执行完成后方法
  var completed = function(completedActionList, args) {
    if (!Array.isArray(completedActionList)) {
      alert(CONFIG_PARAM_ERROR_MSG + '[completed]');
      return;
    }
    
    // 初始化参数（如果任务模块没有返回结果集数组）
    if (!Array.isArray(args)) {
      args = [];
    }
    
    for (var i = 0, c = completedActionList.length; i < c; ++i) {
      var path = completedActionList[i];
 
      // 加载并执行具体模块
      var msg = app.loadScript(path, function(err, cModule) {
         if (err)
           return false;
         return cModule.apply(null, args);
      });
      
      if (msg == false) {
        alert(CONFIG_LOAD_ERROR_MSG + path);
        return;
      }
      // 将完成模块返回结果加入数组
      // 成为下一个完成模块的执行参数
      args.push(msg);
    }
  };

  // 调用自动执行模块
  var automationScript = function(path, runTaskModuleHandle) {
    var msg = app.loadScript(path, function(err, autoFunc) {
      if (err)
        return false;
      autoFunc(runTaskModuleHandle);
      return true;
    });

    if (msg == false) {
      alert(CONFIG_PARAM_ERROR_MSG + "[automation]");
      return;
    }
  };
  
  // gogogo~~~
  initGlobalModule(config.global);
  var taskResult;
  
  var run = function () {
    taskResult = runTaskModule(config.module);
    // 输出报告~~
    completed(config.completed, [taskResult]);
  };

  // 检测是否存在automation配置项
  if(typeof config.automation != 'string') {
    run();
  } else {
    automationScript(config.automation, run);
  }

});