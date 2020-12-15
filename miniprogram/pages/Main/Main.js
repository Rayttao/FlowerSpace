//pages/Main/Main.js
const devicesId = "填写在OneNet上获得的devicesId" // 填写在OneNet上获得的devicesId 形式就是一串数字 例子:9939133
const api_key = "填写在OneNet上的 api-key" // 填写在OneNet上的 api-key 例子: VeFI0HZ44Qn5dZO14AuLbWSlSlI=

Page({

  /**
   * 页面的初始数据
   */
  data: {
      Temperature:0,
      Humidity:0,
      TankWeight:0,
      SoilMoisture:0,
      LightIntensity:0,
      LightIntensity1:0,
      LightFlag:-1,
      WaterFlag:0,
      Days:0
  },
  //下拉页面刷新函数
  onPullDownRefresh: function () {
    wx.showLoading({
      title: "正在获取"
    })
    this.getDatapoints().then(datapoints => {
      this.update(datapoints)
      wx.hideLoading()
    }).catch((error) => {
      wx.hideLoading()
      console.error(error)
    })
  },
  //页面加载函数
  onLoad: function () {
    console.log(`your deviceId: ${devicesId}, apiKey: ${api_key}`)

    //每隔6s自动获取一次数据进行更新
    const timer = setInterval(() => {
      this.getDatapoints().then(datapoints => {
        this.update(datapoints)
      })
    }, 6000)

    wx.showLoading({
      title: '加载中'
    })

    this.getDatapoints().then((datapoints) => {
      wx.hideLoading()
    }).catch((err) => {
      wx.hideLoading()
      console.error(err)
      clearInterval(timer) //首次渲染发生错误时禁止自动刷新
    })
  },
  //获取数据函数
  getDatapoints: function () {
    return new Promise((resolve, reject) => {
      wx.request({
        url: `https://api.heclouds.com/devices/${devicesId}/datapoints?datastream_id=Temperature,Humidity,TankWeight,SoilMoisture,LightIntensity,LightIntensity1,LightFlag,WaterFlag&limit=10`,
        header: {
          'content-type': 'application/json',
          'api-key': api_key
        },
        success: (res) => {
          const status = res.statusCode
          const response = res.data
          if (status !== 200) { // 返回状态码不为200时将Promise置为reject状态
            reject(res.data)
            return ;
          }
          if (response.errno !== 0) { //errno不为零说明可能参数有误, 将Promise置为reject
            reject(response.error)
            return ;
          }
          if (response.data.datastreams.length === 0) {
            reject("当前设备无数据, 请先将硬件设备数据上传")
          }
          
          resolve({
            Temperature: response.data.datastreams[0].datapoints[0].value,
            LightIntensity1:response.data.datastreams[1].datapoints[0].value,
            WaterFlag:response.data.datastreams[2].datapoints[0].value,
            Humidity: response.data.datastreams[3].datapoints[0].value,
            LightFlag:response.data.datastreams[4].datapoints[0].value,
            SoilMoisture:response.data.datastreams[5].datapoints[0].value,
            LightIntensity:response.data.datastreams[6].datapoints[0].value,
            TankWeight:response.data.datastreams[7].datapoints[0].value            
          })
        },
        fail: (err) => {
          reject(err)
        }
      })
    })
  },

  //刷新页面数据函数
  update:function(datapoints){
    const FlowerData = datapoints
    var Day = 3*(FlowerData.TankWeight)/0.2
    Day = Day.toFixed(0)
    this.setData({
      Temperature:FlowerData.Temperature,
      Humidity:FlowerData.Humidity,
      SoilMoisture:FlowerData.SoilMoisture,
      LightIntensity:FlowerData.LightIntensity,
      LightFlag:FlowerData.LightFlag,
      LightIntensity1:FlowerData.LightIntensity1,
      WaterFlag:FlowerData.WaterFlag,
      TankWeight:FlowerData.TankWeight,
      Days:Day
    })

  }
})