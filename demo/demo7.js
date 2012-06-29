// 输出当前目录
var msg = App.process('cmd.exe', [
    '/c', 
    'dir',
    App.path.replace(/\//g, '\\')
  ]);
// 输出文件位置
var log = App.path + 'demo7.txt';
// 写入文件 UTF-8 编码
App.writeFile(log, msg, 'utf-8');
// 输出 c 盘目录
msg = App.process('cmd.exe', ['/c', 'dir', 'c:\\']);
// 以追加方式写入文件
App.writeFile(log, '\n\n' + msg, 'utf-8', true);
// 关闭
App.close();