FROM python:3.5
ADD . /code
WORKDIR /code
RUN pip install uvloop
CMD python -u server_await_uv.py

EXPOSE 1234

