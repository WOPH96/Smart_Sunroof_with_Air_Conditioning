#******************************************************************************
# @file weather_module.py
# @brief Weather API Call
# @author DongHun
# @version 1.0
# @date 2025-01-19
#******************************************************************************

# weather_thread.py
from PyQt5.QtCore import QThread, pyqtSignal
import requests
import time
# from can_module import CANBus

class WeatherThread(QThread):
    weather_updated = pyqtSignal(dict)
    
    def __init__(self, api_key):
        super().__init__()
        self.api_key = api_key
        self.is_running = True

        
    def stop(self):
        self.is_running = False
        
    def fetch_weather_data(self):
        try:
            url = f"http://api.openweathermap.org/data/2.5/weather?q=Seoul&appid={self.api_key}&units=metric"
            response = requests.get(url)
            if response.status_code == 200:
                return response.json()
            return None
        except Exception as e:
            print(f"Error fetching weather data: {e}")
            return None
    
    def run(self):
        while self.is_running:
            weather_data = self.fetch_weather_data()
            
            if weather_data:
                self.weather_updated.emit(weather_data)
            # 1시간 대기
            time.sleep(3600)
