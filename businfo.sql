--
-- ER/Studio 8.0 SQL Code Generation
-- Company :      CHINA
-- Project :      Model2.DM1
-- Author :       USER
--
-- Date Created : Wednesday, March 11, 2015 14:29:41
-- Target DBMS : MySQL 5.x
--

-- 
-- TABLE: bus_base_info 
--

CREATE TABLE bus_base_info(
    license_plate_number    CHAR(20)          NOT NULL,
    own_organize_number     CHAR(20),
    vehicle_model           CHAR(1),
    seats                   INT               NOT NULL,
    factory_plate_number    CHAR(20),
    engine_number           CHAR(20),
    baseplate_type          CHAR(1),
    engine_location         CHAR(8),
    purchasing_date         DATE              NOT NULL,
    purchase_price          DECIMAL(10, 0),
    register_date           DATE              NOT NULL,
    serialnumber            INT               NOT NULL,
    terminal_id             CHAR(20)          NOT NULL,
    comment                 CHAR(50),
    PRIMARY KEY (license_plate_number)
)ENGINE=MYISAM
;



-- 
-- TABLE: data_from_terminal 
--

CREATE TABLE data_from_terminal(
    serialnumber     INT         AUTO_INCREMENT,
    schedule_num     CHAR(10)    NOT NULL,
    terminal_id      CHAR(20)    NOT NULL,
    bus_status       CHAR(1)     NOT NULL,
    longitude        CHAR(4)     NOT NULL,
    latitude         CHAR(4)     NOT NULL,
    num_of_people    CHAR(1)     NOT NULL,
    collect_time     DATETIME    NOT NULL,
    receipttime      DATETIME    NOT NULL,
    PRIMARY KEY (serialnumber)
)ENGINE=MYISAM
;



-- 
-- TABLE: schedule 
--

CREATE TABLE schedule(
    `�Ű���`              CHAR(10)    NOT NULL,
    schedule_num            CHAR(10)    NOT NULL,
    `�ƻ�����ʱ��`          CHAR(10),
    `�Ƿ񷢳�`              CHAR(10),
    `��ʵ����ʱ��`          CHAR(10),
    `����ʱ����`            CHAR(10),
    `����ʱ��Ʊ��`          CHAR(10),
    license_plate_number    CHAR(20),
    `��·���`              CHAR(10)    NOT NULL,
    PRIMARY KEY (`�Ű���`)
)ENGINE=MYISAM
;



-- 
-- TABLE: video_summary_info 
--

CREATE TABLE video_summary_info(
    serialnumber       INT         NOT NULL,
    schedule_num       CHAR(10)    NOT NULL,
    terminal_id        CHAR(20)    NOT NULL,
    FTPserverIPaddr    CHAR(4)     NOT NULL,
    video_name         CHAR(50)    NOT NULL,
    starttime          DATETIME    NOT NULL,
    endtime            DATETIME    NOT NULL,
    receipttime        DATETIME    NOT NULL,
    PRIMARY KEY (serialnumber)
)ENGINE=MYISAM
;



-- 
-- TABLE: `����` 
--

CREATE TABLE `����`(
    `������`  CHAR(10)    NOT NULL,
    `����`      CHAR(10),
    `����`      CHAR(10),
    `���`      CHAR(10),
    `�Ű���`  CHAR(10)    NOT NULL,
    PRIMARY KEY (`������`)
)ENGINE=MYISAM
;



-- 
-- TABLE: `��ʻ��ͣ�켣` 
--

CREATE TABLE `��ʻ��ͣ�켣`(
    `��ʻ��ͣ�켣���`  CHAR(10)    NOT NULL,
    `��㾭��`          CHAR(10),
    `���γ��`          CHAR(10),
    `���ɼ�ʱ��`      CHAR(10),
    `ͣ�㾭��`          CHAR(10),
    `ͣ��γ��`          CHAR(10),
    `ͣ��ɼ�ʱ��`      CHAR(10),
    `��ʻ�����`        CHAR(10),
    `��������`          CHAR(10),
    `ÿ�������`        CHAR(10),
    `�ϼƷ���`          CHAR(10),
    `�Ű���`          CHAR(10)    NOT NULL,
    PRIMARY KEY (`��ʻ��ͣ�켣���`)
)ENGINE=MYISAM
;



-- 
-- TABLE: `��ʻ��` 
--

CREATE TABLE `��ʻ��`(
    `��ʻԱ���`  CHAR(10)    NOT NULL,
    `�Ű���`    CHAR(10)    NOT NULL,
    `��ע`        CHAR(10),
    PRIMARY KEY (`��ʻԱ���`, `�Ű���`)
)ENGINE=MYISAM
;



-- 
-- TABLE: `��ʻԱ��Ϣ` 
--

CREATE TABLE `��ʻԱ��Ϣ`(
    `��ʻԱ���`        CHAR(10)    NOT NULL,
    `״̬`              CHAR(10),
    `��ʻ֤����`        CHAR(10)    NOT NULL,
    `����`              CHAR(10),
    `����`              CHAR(10),
    `�Ա�`              CHAR(10),
    `��ʻ֤���`        CHAR(10),
    `������֤ʱ��`      CHAR(10),
    `��ʻ֤����ʱ��`    CHAR(10),
    `��ҵ�ʸ�֤���`    CHAR(10),
    `��ҵ�ʸ�֤����`    CHAR(10),
    `׼�ݳ���`          CHAR(10),
    `��ҵ�ʸ�����ʱ��`  CHAR(10),
    `��������ʱ��`      CHAR(10),
    PRIMARY KEY (`��ʻԱ���`)
)ENGINE=MYISAM
;



-- 
-- TABLE: `��ɼ�¼` 
--

CREATE TABLE `��ɼ�¼`(
    `��ɼ�¼���`  CHAR(10)    NOT NULL,
    `���ʱ��`      CHAR(10),
    `��ɽ��`      CHAR(10),
    `���ƾ֤��`    CHAR(10),
    `�����`        CHAR(10),
    `��ע`          CHAR(10),
    `�Ű���`      CHAR(10)    NOT NULL,
    PRIMARY KEY (`��ɼ�¼���`)
)ENGINE=MYISAM
;



-- 
-- TABLE: `����ͳ��` 
--

CREATE TABLE `����ͳ��`(
    `����ͳ�Ʊ��`  CHAR(10)    NOT NULL,
    `ͳ��ʱ��`      CHAR(10),
    `����������`    CHAR(10),
    `�Ű���`      CHAR(10)    NOT NULL,
    PRIMARY KEY (`����ͳ�Ʊ��`)
)ENGINE=MYISAM
;



-- 
-- TABLE: `��·�켣` 
--

CREATE TABLE `��·�켣`(
    `�켣���`  CHAR(10)    NOT NULL,
    `�켣���`  CHAR(10),
    `�ص�����`  CHAR(10),
    `��㾭��`  CHAR(10),
    `���γ��`  CHAR(10),
    `�յ�����`  CHAR(10),
    `�յ㾭��`  CHAR(10),
    `�յ�γ��`  CHAR(10),
    `�����`    CHAR(10),
    `Ʊ��`      CHAR(10),
    `��ע`      CHAR(10),
    `��·���`  CHAR(10),
    PRIMARY KEY (`�켣���`)
)ENGINE=MYISAM
;



-- 
-- TABLE: `��·��Ϣ` 
--

CREATE TABLE `��·��Ϣ`(
    `��·���`  CHAR(10)    NOT NULL,
    `���`      CHAR(10),
    `�յ�`      CHAR(10),
    `��·���`  CHAR(10),
    `��ע`      CHAR(10),
    PRIMARY KEY (`��·���`)
)ENGINE=MYISAM
;



-- 
-- TABLE: schedule 
--

ALTER TABLE schedule ADD CONSTRAINT Refbus_base_info5 
    FOREIGN KEY (license_plate_number)
    REFERENCES bus_base_info(license_plate_number)
;

ALTER TABLE schedule ADD CONSTRAINT `Ref��·��Ϣ16` 
    FOREIGN KEY (`��·���`)
    REFERENCES `��·��Ϣ`(`��·���`)
;


-- 
-- TABLE: `����` 
--

ALTER TABLE `����` ADD CONSTRAINT Refschedule21 
    FOREIGN KEY (`�Ű���`)
    REFERENCES schedule(`�Ű���`)
;


-- 
-- TABLE: `��ʻ��ͣ�켣` 
--

ALTER TABLE `��ʻ��ͣ�켣` ADD CONSTRAINT Refschedule18 
    FOREIGN KEY (`�Ű���`)
    REFERENCES schedule(`�Ű���`)
;


-- 
-- TABLE: `��ʻ��` 
--

ALTER TABLE `��ʻ��` ADD CONSTRAINT `Ref��ʻԱ��Ϣ14` 
    FOREIGN KEY (`��ʻԱ���`)
    REFERENCES `��ʻԱ��Ϣ`(`��ʻԱ���`)
;

ALTER TABLE `��ʻ��` ADD CONSTRAINT Refschedule15 
    FOREIGN KEY (`�Ű���`)
    REFERENCES schedule(`�Ű���`)
;


-- 
-- TABLE: `��ɼ�¼` 
--

ALTER TABLE `��ɼ�¼` ADD CONSTRAINT Refschedule20 
    FOREIGN KEY (`�Ű���`)
    REFERENCES schedule(`�Ű���`)
;


-- 
-- TABLE: `����ͳ��` 
--

ALTER TABLE `����ͳ��` ADD CONSTRAINT Refschedule19 
    FOREIGN KEY (`�Ű���`)
    REFERENCES schedule(`�Ű���`)
;


-- 
-- TABLE: `��·�켣` 
--

ALTER TABLE `��·�켣` ADD CONSTRAINT `Ref��·��Ϣ17` 
    FOREIGN KEY (`��·���`)
    REFERENCES `��·��Ϣ`(`��·���`)
;


