@ECHO OFF

:: UPDATE WITH LOCAL PATH(s)
SET MSBUILD=C:\Program Files (x86)\Microsoft Visual Studio\2019\Community\MSBuild\Current\Bin\amd64\MSBuild.exe

CALL "%MSBUILD%" .\csharp_example\csharp_example.sln /p:Configuration=Release /p:Platform=x64


