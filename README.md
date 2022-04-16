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

### bird's eye view -\> **[sliding window](https://www.google.com/url?sa=i&url=https%3A%2F%2Fwww.mdpi.com%2F1424-8220%2F19%2F14%2F3166&psig=AOvVaw3dgcm4vjtKvEwXFY-1ojXB&ust=1649769511140000&source=images&cd=vfe&ved=0CAsQjhxqFwoTCJjaxPGRjPcCFQAAAAAdAAAAABAJ)**

<img src="https://www.mdpi.com/sensors/sensors-19-03166/article_deploy/html/images/sensors-19-03166-g012.png" width="50%">

<br>

- 알고리즘 순서
1. ROI 설정
2. perspective transform
3. hsv -\> split and using only `V`
4. inverse
5. brightness processing
6. gaussian
7. inRange
8. histogram -\> argmax abount left and right -\> sliding

<br>

<br>

---

### Hough transform + **RANSAC**

<img src="https://ars.els-cdn.com/content/image/1-s2.0-S0045790620305085-gr1.jpg" width="50%">
<img src="https://user-images.githubusercontent.com/33013780/162755205-554cf4b9-cc64-40a8-b084-8854fbfb184b.png" width="40%">

<br>

<br>

---

### [Deep learnging](https://encrypted-tbn0.gstatic.com/images?q=tbn:ANd9GcTMCl79-Jxmus3idtZDypeyTOc4ss5H96VjsQ&usqp=CAU) + RANSAC 

<img src="https://encrypted-tbn0.gstatic.com/images?q=tbn:ANd9GcTMCl79-Jxmus3idtZDypeyTOc4ss5H96VjsQ&usqp=CAU">

- [instance segmentation](https://www.google.com/url?sa=i&url=https%3A%2F%2Fpaperswithcode.com%2Fpaper%2Ftowards-end-to-end-lane-detection-an-instance&psig=AOvVaw3dgcm4vjtKvEwXFY-1ojXB&ust=1649769511140000&source=images&cd=vfe&ved=0CAsQjhxqFwoTCJjaxPGRjPcCFQAAAAAdAAAAABAy)

<br>

<br>

---

### [V-ROI](https://github.com/Yeowoolee/OpenCV-Lane-Detection)

<img src="https://user-images.githubusercontent.com/33013780/162750624-38287654-3b98-4132-a8ed-d54cf0672087.png" width="300px"> 
<img src="https://user-images.githubusercontent.com/33013780/162751237-760413eb-4d25-44b7-8c8e-f6c69a116dac.png" width="300px">


- https://yeowool0217.tistory.com/558?category=803755

<br>

<br>

# 중간점검

슬라이딩 윈도우를 기반으로 차선을 인식하고 있다. 그러나 슬라이딩 윈도우가 곡선을 잘 인식하지 못하고 있어서 이를 해결할 방법을 고안해내고자 한다. 현재의 warp된 영상의 크기는 원래 크기의 1/2 크기인 320x240이다. 그러나 공지에 올라온 영상을 보니 warp된 영상의 크기가 매우 큰 것을 보아 이 warp된 영상의 크기를 훨씬 크게 키워서 슬라이딩 윈도우를 한다면 더 정확하고 robust한 차선 탐지가 가능하지 않을까 생각한다.

그 후 슬라이딩 윈도우에서 차선을 인식하지 못하는 경우가 많은 것 같아서 이를 해결하기 위해 인식하지 못했을 때 반대 차선을 대칭으로 그려서 윈도우를 그린다던지, 지난번의 윈도우를 기억하고 있다가 계속 사용하는 방법이 있어서 이를 고안해내고자 한다.

그래서 업무 분담을 위해 각자 하나씩 기능을 맡기로 했다.
- 이현수 : 반대 차선의 윈도우 들을 차선 중앙에 대칭시켜 임의로 예측시킨다.
- 윤재호 : 지난 프레임에서 탐지해놓은 윈도우 들을 기억하고 있다가 차선이 탐지되지 않을 때 지난 것들을 사용한다.

그래서 이 분담을 깃허브에 브랜치를 생성하여 협업을 해보고자 한다.
-\> 최대한 깃 브랜치를 많이 생성하여 사용했다.

현재 슬라이딩 윈도우를 진행할 때 라인을 탐색할 위치를 window 높이에서 4픽셀 아래에서 탐지를해서 다음 윈도우의 위치를 지정해주는 방식이다. 음.. 처음부터 위치 잡고 탐색 후에 그 탐색된 위치에 박스를 그리면 되지 않나?

-\> 그래서 해당 위치에서 탐색 후 박스를 그렸다.

---

# 마무리 및 회고

1. 슬라이딩 윈도우를 통한 ROI 설정
2. warpPerspective
3. HSV를 통한 이진화
  - V 평면만 사용
  - 반전을 통해 더 잘 탐지
  - 평균 밝기 유지
4. 슬라이딩 윈도우 사용
5. 첫 시작점을 잡아줄 때, max_element를 사용하면 동일한 크기의 픽셀 중 가장 앞의 위치를 추출하기에 다른 방법을 사용
  - 초기값 설정을 위해 왼쪽은 0, 오른쪽은 ROI 가로 사이즈인 960으로 지정
  - 255인 위치 중 위치 지정이 안되어 있다면 해당 인덱스를 지정
  - 해당 인덱스가 지정된 후부터 255가 끝날 때까지 끝 점을 지정해줌
  - 255로 되어있다가 0이 되면 if문이 종료된다.
6. 차선의 인덱스들을 통해 중앙값을 차선의 중앙으로 설정
7. slidiing window 진행
  - window개수는 최대한 줄여서 변화에 잘 적응할 수 있도록 12개 정도로 지정
  - window 가로와 세로는 유동적인 설정을 위해 직접 지정이 아닌 window 개수에 따라 변하도록 설정
  - margin은 중앙값을 기준으로 가로 길이/2이다. 즉, 중앙값을 입력으로 받았기 때문에 좌우 +- margin하여 window 좌측, 우측 좌표를 지정함
  - 기존의 sliding window는 윈도우를 만들고 나서 해당 윈도우에서 다음 윈도우의 시작 위치를 찾아주었다. 이렇게 하면 변화에 둔해지는 현상 발생
  - 따라서 탐색할 높이인 offset을 먼저 지정해준 후 탐지 하고 나서 그림을 그려준다. 이렇게 하면 변화에 적응을 더 잘 할 뿐더러 코너를 나름 잘 따라감
  - 이 때도, 차선을 찾기 위해 오른쪽 차선의 시작점, 끝점을 탐지하여 지정, 왼쪽 차선의 시작점, 끝점을 탐지하고, 해당 offset에서의 nonzero값을 구해서 임계값보다 높으면 탐지한 결과값으로 진행하고, 임계값보다 낮으면 지난 픽셀의 값을 사용하거나, 반대편 차선의 대칭으로 만들어줌
  - 이렇게 탐지한 왼쪽, 오른쪽 차선과 그로인해 구해진 중앙 차선을 변수에 저장
  - fitline을 통해 직선을 구한다. 이 때 출력되는 값의 0,1번째 값은 단위 벡터, 2,3번째 값은 선들의 중간 픽셀을 출력해준다.
  - 방향을 일정하게 맞추기 위해 무조건 방향벡터에서의 y를 -로 맞춰주었다.
  - 이를 통해 선 위의 점을 추출하고, 단위 벡터는 중앙값을 기준으로 방향이 맞춰져 있기 때문에, h로곱하는 것이 아닌 중앙값에서 h/2\*단위벡터를 구했다.
  - 중앙 픽셀과 끝 픽셀의 차를 통해 맨 아래 점을 찾아서 ROI화면의 맨 위 아래로 직선을 그렸다.
  - 출력값을 통해 ROI를 다시 frame 형태로변환하는 행렬을 통해 원본 frame에 점 좌표를 그리거나, 직선을 그리고자 햇지만, 아직 수행하지 못했다.
  - ROI를 파란색 선이 포함되도록 자르면 차가 조금만 움직여도 차선을 탐지하기 어려워지기 때문에 높이를 395에서 잘라서 ROI를 만들었다.
  - 아까 그린 좌우 차선의 직선 색을 통해 차선 중앙의 x좌표를 구하고 csv파일로 입력한다.


---


- 개발 언어

<code><img alt = "3.1 Python" height="20" src="https://cdn.icon-icons.com/icons2/2415/PNG/512/c_original_logo_icon_146611.png"> C++ </code> 
