# Installs the dependencies for the REST API
# Usage:
#   sudo ./install_dependencies.sh
#
# Todo: After https://github.com/opencog/opencog/issues/337 is addressed, these dependencies will be
#       handled automatically using requirements.txt

# Required:
easy_install Flask
easy_install -U mock
easy_install -U flask-restful
easy_install six

# Optional:
easy_install requests
