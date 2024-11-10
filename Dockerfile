FROM mcr.microsoft.com/windows/servercore:ltsc2022

# install package manager
RUN powershell -NoProfile -ExecutionPolicy Bypass -Command \
    Invoke-WebRequest https://chocolatey.org/install.ps1 -OutFile install.ps1; \
    PowerShell -NoProfile -ExecutionPolicy Bypass -File install.ps1; \
    Remove-Item -Force install.ps1

# install mingw and openssl
RUN choco install mingw -y
RUN choco install openssl -y 

# add mingw and openssl to path
RUN setx PATH "%PATH%;C:\\mingw\\bin;C:\\ProgramData\\chocolatey\\lib\\openssl\\tools\\openssl\\bin"


ENV OPENSSL_INCLUDE_DIR=C:/ProgramData/chocolatey/lib/openssl/tools/openssl/include
ENV OPENSSL_LIB_DIR=C:/ProgramData/chocolatey/lib/openssl/tools/openssl/lib


WORKDIR /app

# copy src code
COPY ./server /app/server
COPY ./client /app/client
COPY ./crypto /app/crypto

# compile
RUN g++ /app/server/server.cpp /app/crypto/crypto.cpp -o /app/server.exe \
    -I"${OPENSSL_INCLUDE_DIR}" \
    -L"${OPENSSL_LIB_DIR}" \
    -lssl -lcrypto -lws2_32

RUN g++ /app/client/client.cpp /app/crypto/crypto.cpp -o /app/client.exe \
    -I"${OPENSSL_INCLUDE_DIR}" \
    -L"${OPENSSL_LIB_DIR}" \
    -lssl -lcrypto -lws2_32

# define entry point
ARG SERVICE=server
ENTRYPOINT ["cmd", "/c", "C:/app/%SERVICE%.exe"]

# Default CMD to run server, can be overridden when building the image
CMD ["server"]
