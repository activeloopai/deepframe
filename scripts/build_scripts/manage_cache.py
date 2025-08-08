#!/usr/bin/env python3

import sys
import os
import platform
import json
import boto3
from botocore.exceptions import ClientError, BotoCoreError

CACHE_BUCKET = os.environ.get("ACTIVELOOP_CACHE_BUCKET", "activeloop-platform-tests")
CACHE_PATH = os.environ.get("VCPKG_CACHE_PATH", "/vcpkg-cache")
ARCH_NAME = platform.machine().lower()

ACCESS_KEY, SECRET_KEY, SESSION_TOKEN = None, None, None

if os.path.isfile(".cloud_creds.json"):
    with open(".cloud_creds.json", "r", encoding="utf-8") as file:
        creds = json.load(file)
    ACCESS_KEY = creds.get("AccessKeyId")
    SECRET_KEY = creds.get("SecretAccessKey")
    SESSION_TOKEN = creds.get("SessionToken")
else:
    ACCESS_KEY = os.getenv("AWS_ACCESS_KEY_ID")
    SECRET_KEY = os.getenv("AWS_SECRET_ACCESS_KEY")
    SESSION_TOKEN = os.getenv("AWS_SESSION_TOKEN")

s3 = boto3.client(
    "s3",
    aws_access_key_id=ACCESS_KEY,
    aws_secret_access_key=SECRET_KEY,
    aws_session_token=SESSION_TOKEN,
)

if ARCH_NAME in ["arm64", "aarch64"]:
    ARCH_NAME = "aarch64"
elif ARCH_NAME in ["amd64", "x86_64"]:
    ARCH_NAME = "x86_64"
else:
    print(f"unsupported architecture: {ARCH_NAME}")
    sys.exit(1)

if sys.argv[2] == "deepframe":
    cache = f"vcpkg_cache_pyvframe_{ARCH_NAME}.zip"
else:
    print("wrong preset")
    sys.exit(1)

if sys.argv[1] == "download":
    try:
        print("\033[33mDownloading vcpkg binary cache\033[00m")
        s3.download_file(CACHE_BUCKET, f"indra/cache/{cache}", f"/{cache}")
        print("\033[33mExtracting vcpkg binary cache\033[00m")
        os.system(f"cd /; unzip -q -o {cache}; rm {cache}")
    except (ClientError, BotoCoreError) as ex:
        print("\033[31mFailed to download or extract vcpkg binary cache\033[00m")
        print(f"\033[31m{ex}\033[00m")
        if not os.path.isdir(CACHE_PATH):
            os.mkdir(CACHE_PATH)
elif sys.argv[1] == "upload":
    try:
        print("\033[33mArchiving vcpkg binary cache\033[00m")
        os.system(f"cd /; zip -q -r {cache} {CACHE_PATH}")
        print("\033[33mUploading vcpkg binary cache\033[00m")
        s3.upload_file(f"/{cache}", CACHE_BUCKET, f"indra/cache/{cache}")
        os.system(f"rm /{cache}")
    except (ClientError, BotoCoreError) as ex:
        print("\033[31mFailed to archive or upload vcpkg binary cache\033[00m")
        print(f"\033[31m{ex}\033[00m")
else:
    print(
        (
            f"wrong command: {sys.argv[1]}, usage: manage_cache.py "
            + "{download|upload} {deepframe}"
        )
    )
    sys.exit(1)
