CREATE USER 'fra_my'@'localhost' IDENTIFIED BY 'free_pass';
CREATE DATABASE fra_my;
GRANT ALL PRIVILEGES ON fra_my.* TO 'fra_my'@'localhost'
