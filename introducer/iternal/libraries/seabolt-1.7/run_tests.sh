#!/usr/bin/env bash

BASE=$(dirname $0)
DATE_FORMAT="%Y-%m-%dT%H:%M:%S"
PASSWORD="password"
TEST_ARGS=$@

BOLTKIT_NOT_AVAILABLE=11
COMPILATION_FAILED=12
SERVER_INSTALL_FAILED=13
SERVER_CONFIG_FAILED=14
SERVER_START_FAILED=15
SERVER_STOP_FAILED=16
SERVER_INCORRECTLY_CONFIGURED=17
TESTS_FAILED=18
PACKAGING_FAILED=19

if [[ -z "${PYTHON}" ]]; then
    PYTHON="python"
fi

if [[ -z "${NEOCTRLARGS}" ]]; then
    NEO4J_VERSION="-e 3.4"
else
    NEO4J_VERSION="${NEOCTRLARGS}"
fi

if [[ -z "${BOLT_PORT}" ]]; then
    BOLT_PORT=7687
fi

if [[ -z "${HTTP_PORT}" ]]; then
    HTTP_PORT=7474
fi

if [[ -z "${HTTPS_PORT}" ]]; then
    HTTPS_PORT=7473
fi

function check_boltkit
{
    echo "Checking boltkit"
    ${PYTHON} -c "import boltkit" 2> /dev/null
    if [ "$?" -ne "0" ]
    then
        echo "FATAL: The boltkit library is not available. Use \`pip install boltkit\` to install."
        exit ${BOLTKIT_NOT_AVAILABLE}
    fi
}

function stop_server
{
    NEO4J_DIR=$1
    echo "-- Stopping server"
    neoctrl-stop "${NEO4J_DIR}"
    if [ "$?" -ne "0" ]
    then
        echo "FATAL: Failed to stop server."
        exit ${SERVER_STOP_FAILED}
    fi
    trap - EXIT
}

function run_tests
{
    NEO4J_VERSION=$1

    SERVER=${BASE}/build/server
    rm -r ${SERVER} 2> /dev/null

    echo "Testing against Neo4j ${NEO4J_VERSION}"

    echo "-- Installing server"
    NEO4J_DIR=$(neoctrl-install ${NEO4J_VERSION} ${SERVER})
    if [ "$?" -ne "0" ]
    then
        echo "FATAL: Server installation failed."
        exit ${SERVER_INSTALL_FAILED}
    fi
    echo "-- Server installed at ${NEO4J_DIR}"

    echo "-- Configuring server to listen for Bolt on port ${BOLT_PORT}"
    neoctrl-configure "${NEO4J_DIR}" dbms.connector.bolt.listen_address=:${BOLT_PORT}
    if [ "$?" -ne "0" ]
    then
        echo "FATAL: Unable to configure server port."
        exit ${SERVER_CONFIG_FAILED}
    fi

    echo "-- Configuring server to listen for HTTPS on port ${HTTPS_PORT}"
    neoctrl-configure "${NEO4J_DIR}" dbms.connector.https.listen_address=:${HTTPS_PORT}
    if [ "$?" -ne "0" ]
    then
        echo "FATAL: Unable to configure server port."
        exit ${SERVER_CONFIG_FAILED}
    fi

    echo "-- Configuring server to listen for HTTP on port ${HTTP_PORT}"
    neoctrl-configure "${NEO4J_DIR}" dbms.connector.http.listen_address=:${HTTP_PORT}
    if [ "$?" -ne "0" ]
    then
        echo "FATAL: Unable to configure server port."
        exit ${SERVER_CONFIG_FAILED}
    fi

    echo "-- Configuring server to accept IPv6 connections"
    neoctrl-configure "${NEO4J_DIR}" dbms.connectors.default_listen_address=::
    if [ "$?" -ne "0" ]
    then
        echo "FATAL: Unable to configure server for IPv6."
        exit ${SERVER_CONFIG_FAILED}
    fi

    echo "-- Setting initial password"
    neoctrl-set-initial-password "${PASSWORD}" "${NEO4J_DIR}"
    if [ "$?" -ne "0" ]
    then
        echo "FATAL: Unable to set initial password."
        exit ${SERVER_CONFIG_FAILED}
    fi

    echo "-- Starting server"
    NEO4J_BOLT_URI=$(neoctrl-start ${NEO4J_DIR} | grep "^bolt:")
    if [ "$?" -ne "0" ]
    then
        echo "FATAL: Failed to start server."
        exit ${SERVER_START_FAILED}
    fi
    trap "stop_server ${NEO4J_DIR}" EXIT
    echo "-- Server is listening at ${NEO4J_BOLT_URI}"

    echo "-- Checking server"
    BOLT_PASSWORD="${PASSWORD}" BOLT_PORT="${BOLT_PORT}" ${BASE}/build/bin/seabolt-cli debug -a "UNWIND range(1, 10000) AS n RETURN n"
    if [ "$?" -ne "0" ]
    then
        echo "FATAL: Server checks failed."
        exit ${SERVER_INCORRECTLY_CONFIGURED}
    fi

    echo "-- Running tests"
    BOLT_PORT="${BOLT_PORT}" ${BASE}/build/bin/seabolt-test ${TEST_ARGS}
    if [ "$?" -ne "0" ]
    then
        echo "FATAL: Test execution failed."
        exit ${TESTS_FAILED}
    fi

    stop_server "${NEO4J_DIR}"

}

echo "Seabolt test run started at $(date +$DATE_FORMAT)"
check_boltkit
run_tests "${NEO4J_VERSION}"
if [ "$?" -ne "0" ]
then
    echo "FATAL: Test execution failed."
    exit ${TESTS_FAILED}
fi
echo "Seabolt test run completed at $(date +$DATE_FORMAT)"
