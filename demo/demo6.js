// 访问页面
App.webview.open("http://weibo.com/"); 
// 监听文档加载完成
App.webview.addEventListener("load", function(rect) {
  // 截取区域返回 base64
  var base64 = App.webview.dataURIFormRect({x:100,y:100, width:50, height:100});
  // 将此图片加入文档中显示
  App.webview.execScript(function(base64){
    var img = document.createElement("img");
    img.src = base64;
    document.body.appendChild(img);
  }, base64);
});
