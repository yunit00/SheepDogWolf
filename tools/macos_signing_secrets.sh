#!/bin/bash
# Stores the macOS signing secrets the release workflow (.github/workflows/build.yml) uses, on the repository of this
# checkout's `origin`, with the GitHub CLI (gh; logged in as an account that can manage the repository's secrets).
#
#   tools/macos_signing_secrets.sh
#       Exports your newest "Developer ID Application" certificate and its private key from the login keychain (macOS
#       asks for your password for each key), keeps only that one identity, and sets MACOS_CERT_P12 and
#       MACOS_CERT_PASSWORD. Run it again with a new certificate when the old one expires.
#
#   tools/macos_signing_secrets.sh --notary AuthKey_XXXXXXXXXX.p8 KEY_ID ISSUER_ID
#       Sets NOTARY_KEY_P8, NOTARY_KEY_ID and NOTARY_ISSUER_ID from an App Store Connect API key (Users and Access >
#       Integrations > App Store Connect API, role Developer), which notarization uses.
#
#   --dry-run (first argument): does everything but setting the secrets, and says what it would set.
#
# Nothing is left on disk: the exported files live in a temporary folder that is deleted on exit.
set -euo pipefail

DRY_RUN=0
if [ "${1:-}" = "--dry-run" ]; then
    DRY_RUN=1
    shift
fi

REPO="$(git remote get-url origin | sed -E 's#(git@github.com:|https://github.com/)##; s#\.git$##')"
OPENSSL=/usr/bin/openssl # macOS's LibreSSL: its .p12 files are the kind `security import` reads

set_secret() { # name, value on stdin
    if [ "$DRY_RUN" = 1 ]; then
        echo "  would set $1 on $REPO ($(wc -c | tr -d ' ') bytes)"
    else
        gh secret set "$1" -R "$REPO"
        echo "  set $1 on $REPO"
    fi
}

if [ "${1:-}" = "--notary" ]; then
    [ $# -eq 4 ] || { echo "usage: $0 [--dry-run] --notary AuthKey_XXXXXXXXXX.p8 KEY_ID ISSUER_ID" >&2; exit 1; }
    grep -q "BEGIN PRIVATE KEY" "$2" || { echo "$2 is not an App Store Connect .p8 key" >&2; exit 1; }
    base64 -i "$2" | set_secret NOTARY_KEY_P8
    printf %s "$3" | set_secret NOTARY_KEY_ID
    printf %s "$4" | set_secret NOTARY_ISSUER_ID
    exit 0
fi

TMP="$(mktemp -d)"
trap 'rm -rf "$TMP"' EXIT
chmod 700 "$TMP"
cd "$TMP"

# every identity (certificate + private key) of the login keychain; then only the Developer ID Application one
EXPORT_PW="$($OPENSSL rand -hex 16)"
echo "Exporting the signing identities from the login keychain: macOS asks for your password, once per key."
security export -k "$HOME/Library/Keychains/login.keychain-db" -t identities -f pkcs12 -P "$EXPORT_PW" -o all.p12
$OPENSSL pkcs12 -in all.p12 -passin "pass:$EXPORT_PW" -nokeys -out certs.pem 2>/dev/null
$OPENSSL pkcs12 -in all.p12 -passin "pass:$EXPORT_PW" -nocerts -nodes -out keys.pem 2>/dev/null
rm all.p12

# one file per certificate / key
awk '/BEGIN CERTIFICATE/{n++} n{print > ("cert_" n ".pem")}' certs.pem
awk '/BEGIN .*PRIVATE KEY/{n++} n{print > ("key_" n ".pem")}' keys.pem
rm certs.pem keys.pem

# the Developer ID Application certificate that expires last
CERT=""
CERT_END=0
for c in cert_*.pem; do
    $OPENSSL x509 -in "$c" -noout -subject | grep -q "Developer ID Application" || continue
    end="$(date -j -f "%b %e %T %Y %Z" "$($OPENSSL x509 -in "$c" -noout -enddate | cut -d= -f2)" +%s)"
    if [ "$end" -gt "$CERT_END" ]; then
        CERT="$c"
        CERT_END="$end"
    fi
done
[ -n "$CERT" ] || { echo "no Developer ID Application certificate with a private key in the login keychain" >&2; exit 1; }
[ "$CERT_END" -gt "$(date +%s)" ] || { echo "the newest Developer ID Application certificate has expired" >&2; exit 1; }

# its private key: the one with the same public key
CERT_PUB="$($OPENSSL x509 -in "$CERT" -noout -pubkey | $OPENSSL md5)"
KEY=""
for k in key_*.pem; do
    if [ "$($OPENSSL pkey -in "$k" -pubout 2>/dev/null | $OPENSSL md5)" = "$CERT_PUB" ]; then
        KEY="$k"
        break
    fi
done
[ -n "$KEY" ] || { echo "the private key of the Developer ID certificate was not exported" >&2; exit 1; }

echo "Using: $($OPENSSL x509 -in "$CERT" -noout -subject | sed 's/^subject= *//')"
echo "       expires $($OPENSSL x509 -in "$CERT" -noout -enddate | cut -d= -f2)"

# that identity alone, under a new password
P12_PW="$($OPENSSL rand -hex 24)"
$OPENSSL pkcs12 -export -inkey "$KEY" -in "$CERT" -name "Developer ID Application" -passout "pass:$P12_PW" -out signing.p12
rm cert_*.pem key_*.pem

base64 -i signing.p12 | set_secret MACOS_CERT_P12
printf %s "$P12_PW" | set_secret MACOS_CERT_PASSWORD
echo "Done. Notarization also needs the App Store Connect key: $0 --notary AuthKey_XXXXXXXXXX.p8 KEY_ID ISSUER_ID"
