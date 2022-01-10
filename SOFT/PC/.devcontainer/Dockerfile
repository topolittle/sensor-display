ARG PYTHON_VERSION
FROM python:${PYTHON_VERSION}-alpine

ARG USER_NAME

# Install some usefull Python tools for development
# pylint: python code static checker
# autopep8: A tool that automatically formats Python code to conform to the PEP 8 style guide
# yapf: A formatter for Python code
# mypy: Optional static typing for Python
# pydocstyle: Python docstring style checker
# pycodestyle: Python style guide checker
# bandit: Security oriented static analyser for python code
RUN pip3 install --disable-pip-version-check --force-reinstall \
    pylint \
    autopep8 \
    yapf \
    mypy \
    pydocstyle \
    pycodestyle \
    bandit

# Install native tools usefull for development
# git: If I have to explain what it is, please close your editor and do something else
# openssh: Required for Git to perform push/pull/fetch avec SSH repo
# bash: More convenient than Busybox's ASH
# bash-completion: Usefull auto complete for Git and other Bash program
RUN apk add --no-cache \
    git \
    openssh \
    bash \
    bash-completion

# Create the user, set bash as the default shell and make it sudoer without password inside the Docker
# Adds the user to the group that the group id corresponds to the host 'dialout' group id.
# If the group id doesn't exists locally, create a new group with the right group id and add the user to it.
COPY ./envfile /root/
RUN apk add --no-cache sudo \
    && . /root/envfile \
    && rm -f /root/envfile \
    && adduser -D -u $USER_UID $USER_NAME -s /bin/bash \
    && addgroup $USER_NAME wheel \
    && sed -e 's;^# \(%wheel.*NOPASSWD.*\);\1;g' -i /etc/sudoers \
    && if grep :${DIALOUT_GID}: /etc/group; then \
        adduser $USER_NAME $(grep :${DIALOUT_GID}: /etc/group | cut -d ':' -f 1); \
    else \
        addgroup -g ${DIALOUT_GID} vscodedialout && adduser $USER_NAME vscodedialout; \
    fi

# Add the 'll' alias for the user and bash completion for git
RUN echo "alias ll='ls -ltr'" >> /home/$USER_NAME/.bashrc \
    && echo ". /usr/share/bash-completion/completions/git" >> /home/vscode/.bashrc

# Set the default user
USER $USER_NAME

# Project specific installation
RUN sudo pip3 install --disable-pip-version-check --force-reinstall \
    pyserial \
    nvidia-ml-py
