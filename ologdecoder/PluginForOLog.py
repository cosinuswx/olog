#encoding=utf-8

import sublime, sublime_plugin
import subprocess
import re
from subprocess import Popen, PIPE, STDOUT
import time, os, struct
import xml.dom.minidom 
import platform
import codecs
from datetime import datetime

###############################################################################
# 执行指定的命令，输入由命令自己处理
def run_command(strCmd):
	out = ''
	
	try:
		if platform.system() == 'Windows':
			p = subprocess.check_output(strCmd)
			print(p)
		else:
			p = Popen(['/bin/sh', '-c', strCmd], stdout=PIPE, stdin=PIPE, stderr=PIPE)
			out, err = p.communicate()
			if err.decode() != '':
				sublime.error_message('Error when run command: \n' + err.decode() + out.decode())
				return
	except Exception as e:
		print(e)
		sublime.error_message('Error when run command\n')
		return


###############################################################################

###############################################################################
class CustomFilterCommand(sublime_plugin.TextCommand):
	# 分发命令
	def dispatch_command(self, edit, expr, inplace):
		args = expr.split(' ')
		if len(args) <= 0:
			return

		if 'xf' == args[0]:
			self.olog_filter(args, inplace)
		elif 'decodeolog' == args[0]:
			self.decode_olog(args, inplace)


	# 根据参数，处理olog的过滤
	def olog_filter(self, args, inplace):
		lvl, tags, txts = self.olog_analyze_arg(args)

		r = sublime.Region(0, self.view.size())
		content = self.view.substr(r)

		index = 0
		s = ''
		while (-1 != index):
			newIndex = content.find('\n', index)
			if (-1 == newIndex): break

			line = content[index:newIndex]
			items = line.split(']')

			if (len(items) >= 5):
				txtValue = items[4]
				for i in range(5, len(items)):
					txtValue = txtValue + ']' + items[i]

				print(txtValue)
				if (self.match_log_level(items[0], lvl) and
					self.match_regular_list(items[2], tags) and
					self.match_regular_list(txtValue, txts)):
					s = s + '\n'
					s = s + line

			index = newIndex + 1

		self.output_to_view(s, inplace)


	# 判断字符串是否匹配正则表达式中的某一个
	def match_regular_list(self, logStr, reLst):
		if len(reLst) == 0: return True

		realStr = logStr[1:]
		for reItem in reLst:
			if re.match(reItem, realStr):
				return True

		return False


	# 判断日志级别是否匹配
	def match_log_level(self, logLvl, lvl):
		if len(logLvl) != 2: return False

		lvls = ['v', 'd', 'i', 'w', 'e']
		findLvl = False
		curLvl = logLvl[1].lower()
		for lvlItem in lvls:
			if lvlItem == lvl: findLvl = True
			if findLvl and curLvl == lvlItem:
				return True

		return False


	# 解析输入的参数
	def olog_analyze_arg(self, args):
		lvl = 'v'
		tags = []
		txts = []

		for itemArg in args:
			items = itemArg.split(':')
			if (len(items) != 2): continue

			if (items[0] == 'lvl'):
				if (len(items[1]) != 1): continue
				lvl = items[1].lower()
			elif (items[0] == 'tag'):
				tags = items[1].split('|')
			elif (items[0] == 'txt'):
				txts = items[1].split('|')

		return lvl, tags, txts


	# 运行命令，输入是当前view中的内容，并将结果替换到当前view中
	def run_command_and_replace(self, strCmd, inplace):
		r = sublime.Region(0, self.view.size())
		content = self.view.substr(r)

		out = ''
		try:
			p = Popen(['/bin/sh', '-c', strCmd], stdout=PIPE, stdin=PIPE, stderr=PIPE)
			out, err = p.communicate(input=content.encode())
			if err.decode() != '':
				sublime.error_message('Error when run command: \n' + err.decode() + out.decode())
				return
		except Exception as e:
			print(e)
			sublime.error_message('Error when run command\n')
			return

		self.output_to_view(out.decode(), inplace)


	# 将内容输出到view中
	def output_to_view(self, txt, inplace):
		if inplace == False:
			new_view = self.view.window().new_file()
			new_view.run_command('replace_text', {'args' : txt, 'inplace' : 'yes' if (inplace == True) else 'no'})
		else:
			self.view.run_command('replace_text', {'args' : txt, 'inplace' : 'yes' if (inplace == True) else 'no'})


	# 显示输入框，用来输入命令
	def run(self, edit, **args):
		def onDone(expr):
			self.dispatch_command(edit, expr, True if (args['inplace'] == 'yes') else False)

		if (args['source'] == 'commands'):
			self.dispatch_command(edit, args['cmd_line'], False)
		else:
			self.view.window().show_input_panel('custom command:', '', onDone, None, None)

###############################################################################


###############################################################################
# 用来替换文本的命令
# 参数inplace：yes代表在当前文件中显示过滤后的文本，no代表新建一个文件，然后替换文本
class ReplaceTextCommand(sublime_plugin.TextCommand):
	def run(self, edit, **args):
		inplace = args['inplace'].lower()
		if inplace == 'yes':
			self.view.replace(edit, sublime.Region(0, self.view.size()), args['args'])
		else:
			self.view.insert(edit, 0, args['args'])

###############################################################################

###############################################################################
class AutoDecodeologListener(sublime_plugin.EventListener):
	def on_load(self, view):
		self.decode_olog(view.file_name())

	# 解码olog文件
	def decode_olog(self, filename):
		if (filename[filename.rfind('.')+1:] != 'olog'): return

		tempFileName = filename + '.temp'
		strCmd = 'java -jar "%s/ologdecoder/ologdecoder.jar" "%s" "%s"' % (sublime.packages_path(), filename, tempFileName)
		run_command(strCmd)

		if (os.path.exists(tempFileName)):
			os.remove(filename)
			os.rename(tempFileName, filename)

###############################################################################
