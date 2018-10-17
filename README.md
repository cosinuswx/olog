## 组件使用

### 1. 初始化

首先在 build.gradle 中引入olog组件：

```java
repositories {
    jcenter()
}

dependencies {
    compile 'com.winom:olog:1.0.2'
}
```

因为olog的实现是在JNI层的，并且是在JNI定义的单实例，因此同一个进程只需要进行一次初始化，不同进程需要都进行初始化：

```java
Log.setLogImpl(new LogImpl(Constants.LOG_SAVE_PATH, "sample", ".olog"));
```

*Log.setLogImpl* 是设置日志的实现，默认的实现是一个空实现。*LogImpl* 的第一个参数是日志的保存路径；第二个参数是日志文件的前缀；第三个参数是文件的后缀，因为插件是按照 *.olog* 进行区分的，不能修改该值。

同时提供日志级别以及是否需要输出到 logcat 中：

```java
Log.setLogLevel(Log.LEVEL_VERBOSE);
Log.setLogToLogcat(true);
```

日志级别有：LEVEL_VERBOSE、LEVEL_DEBUG、LEVEL_INFO、LEVEL_WARNING、LEVEL_ERROR，和android的日志级别一致。*setLogToLogcat* 建议只在DEBUG模式下设置为true，正式发布设置为false，并提供动态修改的入口。

### 2. 使用

然后就可以直接在代码中像android原生的Log一样使用了，只不过导入的 Log 来自 *com.winom.olog* 而不再是 *android.util.log* 了：

```java
Log.v(TAG, "cost: " + (System.currentTimeMillis() - startTick));
Log.d(TAG, "thread[%d] end", Thread.currentThread().getId());
Log.i(TAG, "startRecord");
Log.w(TAG, "Size mismatch, resetting");
Log.e(TAG, "Class Forname failed : " + e.getMessage());
```

日志文件解密过后的内容如下：

```
[V][2017-03-01 13:50:37.921][MainActivity][1][cost: 6
[D][2017-03-01 13:50:37.922][MainActivity][1][thread[1] end
[I][2017-03-01 13:50:37.922][MainActivity][1][startRecord
[W][2017-03-01 13:50:37.923][MainActivity][1][Size mismatch, resetting
[E][2017-03-01 13:50:37.923][MainActivity][1][Class Forname failed : convert failed
```

第一个方框为日志级别，第二个为日志的时间（按照北京时间转换的），第三个为TAG，第四个为输出日志的线程ID，最后一个为日志的内容。

### 3. 反初始化

日志组件原本应该一直使用着，不需要反初始化。但是实现中使用了内存缓存，因此在crash的时候，需要将日志反初始化，日志组件会将缓存写入到文件中：

```java
Log.uninitLog();
```

注: 建议在crash的时候，将堆栈写入到日志文件中，这样在查问题的时候，能够更好地看到堆栈周围发生了什么。

## 插件使用（Mac OS X && Windows）

### 自动解密
1. 打开Sublime Text 3，点击Sublime Text -> Perferences -> Browse Packages，打开插件所在的目录，然后将 [源码](https://github.com/kevin-nazgul/olog) 中的ologdecoder目录复制到此目录。
2. 重新打开Sublime Text 3，将olog文件直接拖进Sublime Text 3即可自动解密

### 过滤已经解密过的xlog文件
1. 按下快捷键，OS X上面：*Command + Shift + x, Command + Shift + f*，Windows上面：*ctrl + shift + x, ctrl + shift + f*。如果需要修改快捷键，对应修改*Default.sublime-keymap*中的内容。
2. 输入命令 *xf [filterspecs]*，filterspecs可以按下面方式指定：*tag:MainActivity*、*lvl:D*、*txt:startRecord*，字符串按照python的正则表达式匹配，多个匹配可以以竖线（|）分割。
