#!/bin/bash

set -eE

TOKEN="$(curl \
  -H "Authorization: Bearer $ACTIONS_ID_TOKEN_REQUEST_TOKEN" \
  "$ACTIONS_ID_TOKEN_REQUEST_URL&audience=sts.amazonaws.com" | yq .value)"

ASSUME_ROLE_ARN=${ASSUME_ROLE_ARN:-arn:aws:iam::067976305224:role/github}

curl 'https://sts.us-east-1.amazonaws.com' \
  -d 'Action=AssumeRoleWithWebIdentity' \
  -d "RoleSessionName=githubactions-indra-$(date +%s)" \
  -d "RoleArn=${ASSUME_ROLE_ARN}" \
  -d 'Version=2011-06-15' \
  -d 'DurationSeconds=7200' \
  -d "WebIdentityToken=${TOKEN}" >/tmp/creds.xml

yq .AssumeRoleWithWebIdentityResponse.AssumeRoleWithWebIdentityResult.Credentials -o json /tmp/creds.xml >.cloud_creds.json
yq .AssumeRoleWithWebIdentityResponse.AssumeRoleWithWebIdentityResult.Credentials -o shell /tmp/creds.xml >.cloud_creds.env

export "AWS_ACCESS_KEY_ID=$(yq .AssumeRoleWithWebIdentityResponse.AssumeRoleWithWebIdentityResult.Credentials.AccessKeyId /tmp/creds.xml)"
export "AWS_SECRET_ACCESS_KEY=$(yq .AssumeRoleWithWebIdentityResponse.AssumeRoleWithWebIdentityResult.Credentials.SecretAccessKey /tmp/creds.xml)"
export "AWS_SESSION_TOKEN=$(yq .AssumeRoleWithWebIdentityResponse.AssumeRoleWithWebIdentityResult.Credentials.SessionToken /tmp/creds.xml)"
