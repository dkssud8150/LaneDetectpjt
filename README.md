<div align="center">

# 🛣️ Lane Detection Project

차선 인식 심화 프로젝트

</div>

---

## 목표

처리하기 다소 어려운 영상들을 직접 처리해봄으로써 다양한 환경에서의 영상 처리 완료를 목표로 한다. 처리가 어려운 상황을 선정해서 해당 상황에서도 잘 동작하도록 만들어야 할 것이다. 예를 들어 아래의 [예외 상황](https://github.com/dkssud8150/LaneDetectpjt/edit/jhyoon/README.md#%EC%98%88%EC%99%B8-%EC%83%81%ED%99%A9-%EC%B2%98%EB%A6%AC)에 대해 2~3가지를 선정해서 그에 대해 잘 처리되도록 알고리즘을 짠다. 또는 허프 변환 이외에 다른 알고리즘을 사용해보거나, 부가적인 기능을 새롭게 추가해보면 좋을 것 깉다.

---

<br>

<br>

## 영상 처리

### 카메라 exposure 
노출값을 20,100,150 등으로 설정했을 때 canny edge에 대한 출력값들이 달라진다. 너무 밝게하거나 너무 어둡게하면, 차선이 사라지게 된다. 

<br>

### 영상 처리 과정

#### 이진화 방법

1-1. grayscale
- 원본인 컬러 이미지에서 Gray 영상으로 변환

1-2. Gaussian Blur
- 노이즈를 처리

1-3. canny edge
  - 외곽선 추출
  - lowwer threshold는 upper threshold의 2~3배가 적당

<br>

2-1. Gaussian -\> HSV image 

2-2. `inRange`를 통한 이진화 (Threshold)
  - 이 때, 명도에 대한 V만 사용하여 차선의 색이나 주변 밝기를 지정해줘야 한다.
  - e.g. cv2.inRange(hsv, (0,0,**50**), (255,255,255)) or cv2.inRange(hsv, (0,0,**150**), (255,255,255))

2-3. canny edge

<br>

3-1. Gaussian -\> LAB image

3-2. `inRange`를 통한 이진화 (Threshold)

3-3. Canny edge

<br>

<br>

#### 이진화 후 처리

4. ROI 영역 설정
  - 차선이 존재하는 위치에 지정해야 함, 또 필요없는 부분들을 잘 처리해야 한다.
5. houghlineP로 라인 추출 
  - threshold, maxval 등의 파라미터를 잘 설정해야 함
  - 기울기의 절대값이 너무 작은 건 다른 물체들에 해당할 확률이 크므로 처리 x
7. 오른쪽, 왼쪽 분리
8. 라인들의 대표 직선 찾기
  - 선분의 기울기 평균값, 양끝점 좌표의 평균값을 사용하여 대표직선을 찾는다.
  - 노이즈를 제거해야 멀리 떨어져 있는 이상한 값도 함께 처리하지 않게 된다.
  - 모든 데이터를 반영하여 계산하는 것이 아닌 노이즈 데이터를 찾아 계산에서 제외시켜야 한다.
  - `RANSAC` 알고리즘을 통해 노이즈를 제거한다.
9. offset을 설정하여 차선 인식하고, 차선의 중간값과 화면의 중앙과 비교하여 핸들링

<br>

---

### 예외 상황 처리

- 카메라 영상에서 차선이 잡히지 않는 경우 영상처리를 위한 작업공간인 스크린 사이즈를 확대시킨다.
  - 실제 카메라 영상 크기보다 옆으로 넓어진 가상 스크린을 사용하여 작업
  - e.g. 기존 크기 : 640 x 480, 좌우로 200픽셀씩 확장하여 1040x480
- 한쪽 차선만 보이는 경우(차선이 끊기거나 추출되지 않을 경우)에는 추출한 한쪽 차선을 활용하여 대칭을 맞춰서 예측한다.
- 또는 차선은 연속적인 선이므로 지난번 위치를 재사용
- 새로 찾은 차선의 위치가 너무 많이 차이가 날 경우 갑자기 위치가 크게 바뀔 수 없기 때문에 한계값을 정해 이를 넘어갈 경우 무시하고 지난번 위치를 재사용

---

<br>

<br>

## 사용 알고리즘

1. bird's eye view -\> sliding window
 
<img src="https://www.mdpi.com/sensors/sensors-19-03166/article_deploy/html/images/sensors-19-03166-g012.png" width="50%">

2. Hough transform + **RANSAC**

<img src="https://ars.els-cdn.com/content/image/1-s2.0-S0045790620305085-gr1.jpg" width="50%">

3. Deep learnging + RANSAC

<img src="https://encrypted-tbn0.gstatic.com/images?q=tbn:ANd9GcTMCl79-Jxmus3idtZDypeyTOc4ss5H96VjsQ&usqp=CAU">

4. V-ROI

<img src="https://user-images.githubusercontent.com/33013780/162751237-760413eb-4d25-44b7-8c8e-f6c69a116dac.png" width="300px">
<img src="https://user-images.githubusercontent.com/33013780/162750624-38287654-3b98-4132-a8ed-d54cf0672087.png" width="300px"> 

- https://github.com/Yeowoolee/OpenCV-Lane-Detection
- https://yeowool0217.tistory.com/558?category=803755

<br>

<br>


---

- 개발 언어

<code><img alt = "3.1 Python" height="20" src="https://cdn.icon-icons.com/icons2/2415/PNG/512/c_original_logo_icon_146611.png"> C++ </code> 
