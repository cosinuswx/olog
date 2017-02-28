# sublime text 3的一些插件

**给olog使用的一些插件**

## 使用方法（Mac OS X && Windows）

### 自动解密
1. 打开Sublime Text 3，点击Sublime Text -> Perferences -> Browse Packages，打开插件所在的目录，然后git clone当此目录。
2. 重新打开Sublime Text 3，将olog文件直接拖进Sublime Text 3即可自动解密

### 过滤已经解密过的xlog文件
1. 按下快捷键，OS X上面：`Command + Shift + x, Command + Shift + f`，Windows上面：`ctrl + shift + x, ctrl + shift + f`。如果需要修改快捷键，对应修改*Default.sublime-keymap*中的内容。
2. 输入命令`xf [filterspecs]`，filterspecs可以按下面方式指定：tag:MainActivity、lvl:D、txt:startRecord，字符串按照python的正则表达式匹配，多个匹配可以以`|`分割
