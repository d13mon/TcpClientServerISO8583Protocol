Create user C##SCOTT identified by tiger;
 
GRANT CONNECT, RESOURCE TO C##SCOTT;

ALTER USER c##scott quota unlimited on USERS;

CONNECT C##SCOTT/tiger

CREATE TABLE response_code_settings (
    id NUMBER GENERATED by default on null as IDENTITY,
	input_code VARCHAR2(10) NOT NULL,
	result_code VARCHAR2(10) NOT NULL 
);

INSERT INTO response_code_settings (input_code, result_code) VALUES ('41','05');
INSERT INTO response_code_settings (input_code, result_code) VALUES ('56','05');
INSERT INTO response_code_settings (input_code, result_code) VALUES ('43','63');
INSERT INTO response_code_settings (input_code, result_code) VALUES ('58','57');
INSERT INTO response_code_settings (input_code, result_code) VALUES ('36','62');
INSERT INTO response_code_settings (input_code, result_code) VALUES ('39','76');
INSERT INTO response_code_settings (input_code, result_code) VALUES ('64','76');

commit;