import network
import time
import gc
 
class WifiController(object):
   
	def __init__(self):
		self.__wlan = network.WLAN(network.STA_IF)
		self.__ap = None
   

	def do_sta(self, sta_id, sta_passwd):
		"""
		连接wifi
		:param sta_id: wifi的名称
		:param sta_passwd: wifi的密码
		:return: True成功	False失败
		"""
		#以下为正常的WIFI连接流程
		self.__wlan.active(True) 
		if self.__wlan.isconnected(): 
			self.__wlan.disconnect()
		if not self.__wlan.isconnected(): 
			print('WifiController:connecting to network...')
			# self.__wlan.active(False)
			self.__wlan.connect(sta_id, sta_passwd) 
			import time
			time.sleep(10) #一般睡个5-10秒,应该绰绰有余

			if not self.__wlan.isconnected():
				self.__wlan.active(False) #关掉连接,免得repl死循环输出
				print('WifiController:wifi connection error, please reconnect')
				return False
			else:
				print('WifiController:wifi config:', self.__wlan.ifconfig())
				return True
		
	def close_sta(self):
		self.__wlan.disconnect()
		self.__wlan.active(False)
 
	def create_ap(self, ap_id, ap_passwd=""):
		"""
		打开AP模式，并创建
		:param ap_id: AP的名称
		:param ap_passwd: AP的连接密码(可以为空字符串)
		:return: True
		"""
		import network
		if self.__ap == None:
			self.__ap = network.WLAN(network.AP_IF) # create access-point interface
		
		print('WifiController:AP config:', self.__ap.config("essid"))
		self.__ap.active(True)
		"""
		AUTH_OPEN -- 0
		AUTH_WEP -- 1
		AUTH_WPA_PSK -- 2
		AUTH_WPA2_PSK -- 3
		AUTH_WPA_WPA2_PSK -- 4
		"""
		# 先进行一次认证方式的修改，缺少这一步后面修改将会不生效
		self.__ap.config(essid=ap_id, authmode=network.AUTH_WPA2_PSK)
		#self.__ap.config(password=ap_passwd)
		if ap_passwd.strip() == "":
			self.__ap.config(essid=ap_id, authmode=network.AUTH_OPEN)
		else:
			self.__ap.config(essid=ap_id, authmode=network.AUTH_WPA_WPA2_PSK, password=ap_passwd)
		print('WifiController:AP config:', self.__ap.ifconfig())
		print('WifiController:AP config:', self.__ap.config("essid"))
		#gc.collect()
		return True
		
	def close_ap(self):
		self.__ap.active(False)

	def get_valid_bs_list(self, bs_list):
		"""
		传入欲判断有无效的WiFi名称列表
		:param bs_list: WiFi名称列表
		:return: 有效的wifi列表
		"""
		self.__wlan.active(True) 
		valid_bs_list = []
		search_tuple = self.__wlan.scan() # scan for access points
		#search_list = [ str(each[0], encoding = "utf-8") for each in search_tuple]
		search_list = [ each[0].decode() for each in search_tuple]
		print(search_list)
		for bs in bs_list:
			if bs in search_list:
				valid_bs_list.append(bs)
		gc.collect()
		return valid_bs_list

"""
This is demo
"""
def main():
	wifi = WifiController()
	ret = wifi.get_valid_bs_list(["orangepi"])
	print(ret)
	#ret = wifi.do_sta("Test", "88888888")
	ret = wifi.do_sta("orangepi", "orangepi")
	if ret == True:
		print("do_sta succ")
		wifi.close_sta()
	ret = wifi.create_ap("Heat_Platform", "")
	if ret == True:
		print("do_ap succ")
		wifi.close_ap()
	while True:
	  time.sleep(1)


if __name__ == "test":
#if __name__ == '__main__':
	main()







