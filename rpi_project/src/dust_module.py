#******************************************************************************
# @file dust_module.py
# @brief Dust API Call
# @author DongHun
# @version 1.0
# @date 2025-01-19
#******************************************************************************

from PyQt5.QtCore import QThread, pyqtSignal
import requests
import pandas as pd
import time

class DustThread(QThread):
    dust_updated = pyqtSignal(dict)
    
    def __init__(self, api_key, location):
        super().__init__()
        self.api_key = api_key
        self.location = location
        self.is_running = True
        
    def stop(self):
        self.is_running = False
        
    def fetch_dust_data(self):
        try:
            # 공공데이터 포털 미세먼지 API 엔드포인트
            url = 'http://apis.data.go.kr/B552584/ArpltnInforInqireSvc/getMsrstnAcctoRltmMesureDnsty'
            params = {
            'serviceKey': self.api_key,
            'returnType': 'json',
            'numOfRows': '1',
            'pageNo': '1',
            'stationName':self.location,
            'dataTerm': 'DAILY',
            'ver': '1.0'
            }
            
            response = requests.get(url, params=params)
            
            if response.status_code == 200:
                data = response.json()
                if data['response']['header']['resultCode'] == '00':
                    items = data['response']['body']['items']
                    if items:
                        return {
                            'pm10': items[0]['pm10Value'],  # 미세먼지
                            'pm25': items[0]['pm25Value'],  # 초미세먼지
                            'timestamp': items[0]['dataTime']
                        }
            return None
            
        except Exception as e:
            print(f"Error fetching dust data: {e}")
            return None
            
    def get_dust_status(self, pm10_value):
        """미세먼지 수치에 따른 상태 반환"""
        if int(pm10_value) <= 30:
            # 좋음
            return 1
        elif int(pm10_value) <= 80:
            # 보통
            return 2
        elif int(pm10_value) <= 150:
            # 나쁨
            return 3
        else:
            # 매우 나쁨
            return 4
            
    def run(self):
        while self.is_running:
            dust_data = self.fetch_dust_data()
            
            if dust_data:
                self.dust_updated.emit(dust_data)
            # 1시간 대기
            time.sleep(3600) 
