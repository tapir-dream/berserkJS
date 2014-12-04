# berserkJS（大名：狂暴JS / 昵称：疯子JS）


页面性能分析与测试自动化工具，可用 JS 编写自己的检测、分析规则，并自动执行。

基于 QT 开发，理论上可以跨平台使用，前提是在目标平台编译并部署 QT 运行环境。

此工具用于尝试前端自动化分析页面网络请求数据，可以使用 JS 操作页面导向，获取所需数据。


#使用案例


- **无界面浏览器测试**：在不依赖本地任何浏览器的情况下，运行测试框架，如 QUnit，Capybara, QUnit, Mocha, WebDriver, YUI Test, BusterJS, FuncUnit, Robot Framework 等。
- **页面自动化**：可以无障碍访问和操作网页的标准 DOM API 以及页面所用 JS 变量、对象、属性等内容。
- **屏幕捕获**：以编程方式获取网页全部或部分内容，可根据 Selector 截取指定 DOM 元素渲染情况；包括 CSS，SVG 和 Canvas。可将截取图片 base64 化，以便发送给远端服务器保存。
- **网络监控**：自动化的网络性能监控，跟踪页面所有资源加载情况并可简便的将输出结果格式化为标准HAR格式。
- **页面性能监控**：自动化的页面渲染监控，可获取 CPU、 内存使用情况数据，根据页面整体情况可简便的输出首次渲染时间、首屏渲染时间等关键数据。


#工具特性

- **跨平台性**：基于 Qt 开发，可跨平台编译，部署。内置基于 QtWebkit 的浏览器环境。源码需在目标系统中编译后，可产生运行于 Windows / Linux / Mac 系统的可执行文件。
- **功能性**：工具内置 webkit 浏览器内核，可响应浏览器内核事件回调、支持发送鼠标消息给浏览器、包装浏览器网络请求数据为JS数据格式、可与浏览器内JS做数据交互。
- **开放性**：工具将主要操作均包装为JS语法与数据格式，采用JS语法包装，前端工程师可根据API组装出符合各自预期的检测功能。
- **接口性**：工具本身支持命令行参数，可带参调用。API支持处理外部进程读取输出流、支持HTTP发送数据。可由 WEB 程序远程调用后获取测试的返回结果。
- **标准性**：完全真实的浏览器环境内 DOM，CSS，JavaScript，Canvas，SVG 可供使用，绝无仿真模拟。


#特点差异

与 PhantomJS 相比具有以下不同：
- **API 简易**: 更直接的 API，如获取网络性能数据，仅需 3 行代码，而非 PhantomJS 的几十行，且信息量比 PhantomJS 丰富。
- **API 标准化**： 常用 API 均采用 W3 规范标准命名，事件处理代码可重复绑定而不相互覆盖，可以无缝兼容 Wind.JS 等异步流程处理库来解决自动化时异步流程控制问题。
- **页面性能信息丰富**：具有页面渲染和 CPU、 内存使用情况数据获取能力，可输出首次渲染时间、首屏渲染时间等页面性能关键数据。
- **调试便利**: 具有 GUI 界面与命令行状态两种形式，开发调试期可使用 GUI 模式定位问题，此模式中可开启 WebKit 的 Inspector 工具辅助调试页面代码与 DOM 。实际运行时可开启命令行状态避免自动执行时 GUI 界面干扰。


#应用企业

- **新浪微博**：已使用 berserkJS 构建前端性能监测数据分析平台，防止微博主要产品在不停开发迭代时，页面性能产生退化。
- **Cisco**: 用于 WebEx 项目的自动化测试

#API 页面

- **详情请看文档页**：
http://tapir-dream.github.com/berserkJS

#介绍PPT

- **观看地址**：http://vdisk.weibo.com/s/Des0SwUIlPVp


#如何使用

## Windows
- 直接执行源码包下 build\release\berserkJS.exe

## Mac
1. 下载并安装 Qt libraries 4.8.5 for Mac
2. 执行源码包下 build\mac_64\berserkJS

>【官网地址】 http://qt-project.org/downloads 

>【立即下载】 http://download.qt-project.org/official_releases/qt/4.8/4.8.6/qt-opensource-mac-4.8.6-1.dmg

- 如果需要自己编译则步骤如下：

1. 下载如上Qt 4.8 依赖库并安装
2. 在 src 目录下执行 qmake berserkjs.pro -spce macx-xcode 命令生成 xcode 工程文件
3. 使用 xcode 开启工程文件执行编译

## Linux
1. 下载 Qt libraries 4.8.5 for Linux/X11
2. 确定系统内存在 X11 lib，否则请使用 yum 等工具安装依赖
3. 如果需要在纯命令行下使用，请使用yum 等工具安装 Xvfb 来模拟 X11 环境。
4. 解压 Qt libraries 4.8.5 for Linux/X11，并进入目录
5. 执行 ./configure
6. 执行 make
7. 执行 install
8. 进入 berserkjs 源码包的 src 目录
9. 执行 qmark berserkjs.pro
10. 执行 mark 后可编译出 berserkJS 的可执行文件
11. 执行 berserkJS

>【官网地址】 http://qt-project.org/downloads 

>【立即下载】 http://download.qt-project.org/official_releases/qt/4.8/4.8.5/qt-everywhere-opensource-src-4.8.5.tar.gz

#LICENCE
- 采用 BSD 开源协议，细节请阅读项目中的 LICENSE.BSD 文件内容。
