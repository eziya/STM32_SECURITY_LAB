@echo off
:: ============================================================================
::  prebuild.bat (EWARM / SBSFU Project Pre-Build Script)
::
::  목적 (Purpose):
::    1) se_crypto_config.h 에 정의된 SECBOOT_CRYPTO_SCHEME 매크로 값을 추출
::       - prepareimage (conf 서브커맨드) 사용 → crypto.txt 생성
::    2) 선택된 보안 스킴에 따라 AES 키(.bin) 및/또는 ECC 키(.txt; 현재는 EC PRIVATE KEY PEM 포함)를
::       어셈블리 함수(SE_ReadKey_x / SE_ReadKey_x_Pub) 형태로 se_key.s 에 생성
::       - prepareimage (trans 서브커맨드) 사용
::    3) 선택된 스킴 이름과 동일한 %crypto%.bat 템플릿을 postbuild.bat 으로 복사
::
::  생성/사용 파일 (Artifacts):
::    %1\output.txt    : 로그 (이 스크립트 시작 시 새로 작성 후 append)
::    %1\crypto.txt     : 선택된 SECBOOT_* 스킴 식별자 한 줄
::    %1\se_key.s       : 키를 반환하는 어셈블리 함수/데이터 (SE_ReadKey_*)
::    %1\postbuild.bat  : 스킴별 후속 빌드 단계 스크립트 (%crypto%.bat 복사본)
::
::  prepareimage 서브커맨드:
::    conf  : 헤더에서 매크로 값 추출
::    trans : 키 파일을 읽어 어셈블리 코드 조각(지정 함수명) 출력
::
::  스킴 분기 개요:
::    - SECBOOT_AES128_GCM_AES128_GCM_AES128_GCM
::        → AES(GCM) 키만 1~3개(SE_ReadKey_1..3), ECDSA 공개키 없음
::    - SECBOOT_ECCDSA_WITH_AES128_CBC_SHA256
::        → AES(CBC) 키(SE_ReadKey_1..3) + ECDSA 공개키(SE_ReadKey_n_Pub)
::    - SECBOOT_ECCDSA_WITHOUT_ENCRYPT_SHA256
::        → AES 생략, ECDSA 공개키만 삽입
::
::  다중 키(Company1/2/3 & KEY1/2/3):
::    - 멀티 슬롯 / 롤오버 / 다중 공급사(OEM) / 확장 신뢰 루트 대비 구조
::
::  보안 주의 (중요):
::    - ECCKEY*.txt 는 현재 EC PRIVATE KEY(개인키)를 포함하고 있음.
::    - prepareimage trans 는 이 개인키에서 ‘공개키’를 추출하여 *_Pub 함수 어셈블리에 넣는 용도.
::    - 프로덕션 환경에서는 리포지토리에 개인키를 두지 말고 공개키만 포함시키며, 개인키는 오프라인 서명 전용으로 관리해야 함.
::
::  기타:
::    - 아래의 "echo 	section ..." 줄은 탭 문자가 필요 (IAR 어셈블러 포맷 규칙 유지).
::    - goto exe: 다음에 있는 goto py: 는 실제 실행 흐름상 도달하지 않는 dead code 이지만 원본 구조를 유지함.
:: ============================================================================

echo prebuild.bat : started > %1\output.txt
set "asmfile=%1\se_key.s"

::comment this line to force python
::python is used if windows executable not found

:: prepareimage 유틸리티 경로 계산 (상대 경로 이동 후 basedir 캡처)
pushd %1\..\..\..\..\..\..\Middlewares\ST\STM32_Secure_Engine\Utilities\KeysAndImages
set basedir=%cd%
popd

:: 실행 형태 분기 (exe 우선, 없으면 python) - 두 번째 goto py: 는 도달 불가 (원본 유지)
goto exe:
goto py:

:exe
:: Windows 실행파일 버전 시도
echo Prebuild with windows executable
set "prepareimage=%basedir%\win\prepareimage\prepareimage.exe"
set "python="
if exist %prepareimage% (
goto prebuild
)

:py
:: Python 스크립트 버전 사용
echo Prebuild with python script
set "prepareimage=%basedir%\prepareimage.py"
set "python=python "
echo "python: %prepareimage%" >> %1\output.txt 2>>&1

:prebuild
:: 암호 스킴 설정 헤더
set "crypto_h=%1\..\Inc\se_crypto_config.h"

:: ----------------------------------------------------------------------------
:: clean 단계: 이전 빌드 산출물 제거 (스킴/키/후처리 스크립트 재생성 대비)
:: ----------------------------------------------------------------------------
if exist %1\crypto.txt (
  del %1\crypto.txt
)
if exist %asmfile% (
  del %asmfile%
)
:: Tabulation before section is mandatory !
echo 	section .SE_Key_Data:CODE>%asmfile%

if exist %1\postbuild.bat (
  del %1\postbuild.bat
)

:: ----------------------------------------------------------------------------
:: 스킴 추출: prepareimage conf → crypto.txt (SECBOOT_* 한 줄)
:: command 변수는 오류 발생 시 어떤 명령이 실패했는지 로깅하기 위해 유지
:: ----------------------------------------------------------------------------
set "command=%python%%prepareimage% conf %crypto_h% > %1\crypto.txt"
%command%
IF %ERRORLEVEL% NEQ 0 goto error
set /P crypto=<%1\crypto.txt >> %1\output.txt 2>>&1
echo crypto %crypto% selected >> %1\output.txt 2>>&1

:: 대상 코어(어셈블리 출력 변형 옵션)
set "cortex=V7M"

:: ----------------------------------------------------------------------------
:: 스킴 분기:
::   - AES-only(GCM×3) 또는 ECC+AES 조합은 AES128 라벨 이용
::   - ECDSA only (WITHOUT_ENCRYPT)는 바로 ECDSA 라벨로 이동
:: ----------------------------------------------------------------------------
if "%crypto%"=="SECBOOT_AES128_GCM_AES128_GCM_AES128_GCM" (
  set "type=GCM"
  goto AES128
)
if "%crypto%"=="SECBOOT_ECCDSA_WITH_AES128_CBC_SHA256" (
  set "type=CBC"
  goto AES128
)
if "%crypto%"=="SECBOOT_ECCDSA_WITHOUT_ENCRYPT_SHA256" (
  goto ECDSA
)
goto end

:AES128
:: ----------------------------------------------------------------------------
:: AES 키 삽입:
::   prepareimage trans -k <키파일> -f SE_ReadKey_n -v %cortex%
::   → 어셈블리 함수 SE_ReadKey_n 내에 키 바이트 포함
::   COMPANY1/2/3 순으로 존재 여부 검사 (선택적 다중 키)
:: ----------------------------------------------------------------------------
set "oemkey=%1\..\Binary\OEM_KEY_COMPANY1_key_AES_%type%.bin"
set "command=%python%%prepareimage% trans -k %oemkey% -f SE_ReadKey_1 -v %cortex% >> %asmfile%"
%command%
IF %ERRORLEVEL% NEQ 0 goto error

set "oemkey=%1\..\Binary\OEM_KEY_COMPANY2_key_AES_%type%.bin"
IF NOT EXIST %oemkey% goto AES128_end
set "command=%python%%prepareimage% trans -k %oemkey% -f SE_ReadKey_2 -v %cortex% >> %asmfile%"
%command%
IF %ERRORLEVEL% NEQ 0 goto error

set "oemkey=%1\..\Binary\OEM_KEY_COMPANY3_key_AES_%type%.bin"
IF NOT EXIST %oemkey% goto AES128_end
set "command=%python%%prepareimage% trans -k %oemkey% -f SE_ReadKey_3 -v %cortex% >> %asmfile%"
%command%
IF %ERRORLEVEL% NEQ 0 goto error

:AES128_end
:: AES-only 스킴(GCM 3중)인 경우 ECDSA 단계 없이 종료
if "%crypto%"=="SECBOOT_AES128_GCM_AES128_GCM_AES128_GCM" goto end

:ECDSA
:: ----------------------------------------------------------------------------
:: ECDSA 공개키 삽입:
::   - ECCKEY*.txt 는 현재 EC PRIVATE KEY PEM 포함(교육/예제용)
::   - prepareimage trans 는 파일에서 공개키를 추출하여 SE_ReadKey_n_Pub 생성
::   - 프로덕션: 공개키만 포함된 파일 사용 권장 (개인키 비공개 관리)
:: ----------------------------------------------------------------------------
set "ecckey=%1\..\Binary\ECCKEY1.txt"
set "command=%python%%prepareimage% trans -k %ecckey% -f SE_ReadKey_1_Pub -v %cortex% >> %asmfile%"
%command%
IF %ERRORLEVEL% NEQ 0 goto error

set "ecckey=%1\..\Binary\ECCKEY2.txt"
IF NOT EXIST %ecckey% goto end
set "command=%python%%prepareimage% trans -k %ecckey% -f SE_ReadKey_2_Pub -v %cortex% >> %asmfile%"
%command%
IF %ERRORLEVEL% NEQ 0 goto error

set "ecckey=%1\..\Binary\ECCKEY3.txt"
IF NOT EXIST %ecckey% goto end
set "command=%python%%prepareimage% trans -k %ecckey% -f SE_ReadKey_3_Pub -v %cortex% >> %asmfile%"
%command%
IF %ERRORLEVEL% NEQ 0 goto error

goto end

:end
:: ----------------------------------------------------------------------------
:: 어셈블리 파일 마무리 및 postbuild 스크립트 준비
::   - END : 어셈블리 종료 마커(IAR 등 환경에서 관례적 사용)
::   - %crypto%.bat → postbuild.bat 복사 (스킴별 후속 처리)
:: ----------------------------------------------------------------------------
echo     END >> %asmfile%
set "command=copy %1\%crypto%.bat %1\postbuild.bat"
%command%
IF %ERRORLEVEL% NEQ 0 goto error
exit 0

:error
:: ----------------------------------------------------------------------------
:: 에러 처리:
::   마지막 실행 명령(%command%)를 로그에 남기고 종료
::   pause 로 IDE 빌드 출력 창에서 메시지 확인 가능
:: ----------------------------------------------------------------------------
echo %command% : failed >> %1\output.txt 2>>&1
echo %command% : failed
pause
exit 1
