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
    `排班编号`              CHAR(10)    NOT NULL,
    schedule_num            CHAR(10)    NOT NULL,
    `计划发车时间`          CHAR(10),
    `是否发车`              CHAR(10),
    `真实发车时间`          CHAR(10),
    `发车时人数`            CHAR(10),
    `发车时总票价`          CHAR(10),
    license_plate_number    CHAR(20),
    `线路编号`              CHAR(10)    NOT NULL,
    PRIMARY KEY (`排班编号`)
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
-- TABLE: `罚款` 
--

CREATE TABLE `罚款`(
    `罚款编号`  CHAR(10)    NOT NULL,
    `理由`      CHAR(10),
    `日期`      CHAR(10),
    `金额`      CHAR(10),
    `排班编号`  CHAR(10)    NOT NULL,
    PRIMARY KEY (`罚款编号`)
)ENGINE=MYISAM
;



-- 
-- TABLE: `行驶起停轨迹` 
--

CREATE TABLE `行驶起停轨迹`(
    `行驶起停轨迹编号`  CHAR(10)    NOT NULL,
    `起点经度`          CHAR(10),
    `起点纬度`          CHAR(10),
    `起点采集时间`      CHAR(10),
    `停点经度`          CHAR(10),
    `停点纬度`          CHAR(10),
    `停点采集时间`      CHAR(10),
    `行驶里程数`        CHAR(10),
    `车载人数`          CHAR(10),
    `每公里费用`        CHAR(10),
    `合计费用`          CHAR(10),
    `排班编号`          CHAR(10)    NOT NULL,
    PRIMARY KEY (`行驶起停轨迹编号`)
)ENGINE=MYISAM
;



-- 
-- TABLE: `驾驶人` 
--

CREATE TABLE `驾驶人`(
    `驾驶员编号`  CHAR(10)    NOT NULL,
    `排班编号`    CHAR(10)    NOT NULL,
    `备注`        CHAR(10),
    PRIMARY KEY (`驾驶员编号`, `排班编号`)
)ENGINE=MYISAM
;



-- 
-- TABLE: `驾驶员信息` 
--

CREATE TABLE `驾驶员信息`(
    `驾驶员编号`        CHAR(10)    NOT NULL,
    `状态`              CHAR(10),
    `驾驶证号码`        CHAR(10)    NOT NULL,
    `姓名`              CHAR(10),
    `年龄`              CHAR(10),
    `性别`              CHAR(10),
    `驾驶证类别`        CHAR(10),
    `初次领证时间`      CHAR(10),
    `驾驶证审验时间`    CHAR(10),
    `从业资格证类别`    CHAR(10),
    `从业资格证号码`    CHAR(10),
    `准驾车型`          CHAR(10),
    `从业资格审验时间`  CHAR(10),
    `继续教育时间`      CHAR(10),
    PRIMARY KEY (`驾驶员编号`)
)ENGINE=MYISAM
;



-- 
-- TABLE: `解缴记录` 
--

CREATE TABLE `解缴记录`(
    `解缴记录编号`  CHAR(10)    NOT NULL,
    `解缴时间`      CHAR(10),
    `解缴金额`      CHAR(10),
    `解缴凭证号`    CHAR(10),
    `解缴人`        CHAR(10),
    `备注`          CHAR(10),
    `排班编号`      CHAR(10)    NOT NULL,
    PRIMARY KEY (`解缴记录编号`)
)ENGINE=MYISAM
;



-- 
-- TABLE: `收入统计` 
--

CREATE TABLE `收入统计`(
    `收入统计编号`  CHAR(10)    NOT NULL,
    `统计时间`      CHAR(10),
    `单车月收入`    CHAR(10),
    `排班编号`      CHAR(10)    NOT NULL,
    PRIMARY KEY (`收入统计编号`)
)ENGINE=MYISAM
;



-- 
-- TABLE: `线路轨迹` 
--

CREATE TABLE `线路轨迹`(
    `轨迹编号`  CHAR(10)    NOT NULL,
    `轨迹序号`  CHAR(10),
    `地点名称`  CHAR(10),
    `起点经度`  CHAR(10),
    `起点纬度`  CHAR(10),
    `终点名称`  CHAR(10),
    `终点经度`  CHAR(10),
    `终点纬度`  CHAR(10),
    `里程数`    CHAR(10),
    `票价`      CHAR(10),
    `备注`      CHAR(10),
    `线路编号`  CHAR(10),
    PRIMARY KEY (`轨迹编号`)
)ENGINE=MYISAM
;



-- 
-- TABLE: `线路信息` 
--

CREATE TABLE `线路信息`(
    `线路编号`  CHAR(10)    NOT NULL,
    `起点`      CHAR(10),
    `终点`      CHAR(10),
    `线路类别`  CHAR(10),
    `备注`      CHAR(10),
    PRIMARY KEY (`线路编号`)
)ENGINE=MYISAM
;



-- 
-- TABLE: schedule 
--

ALTER TABLE schedule ADD CONSTRAINT Refbus_base_info5 
    FOREIGN KEY (license_plate_number)
    REFERENCES bus_base_info(license_plate_number)
;

ALTER TABLE schedule ADD CONSTRAINT `Ref线路信息16` 
    FOREIGN KEY (`线路编号`)
    REFERENCES `线路信息`(`线路编号`)
;


-- 
-- TABLE: `罚款` 
--

ALTER TABLE `罚款` ADD CONSTRAINT Refschedule21 
    FOREIGN KEY (`排班编号`)
    REFERENCES schedule(`排班编号`)
;


-- 
-- TABLE: `行驶起停轨迹` 
--

ALTER TABLE `行驶起停轨迹` ADD CONSTRAINT Refschedule18 
    FOREIGN KEY (`排班编号`)
    REFERENCES schedule(`排班编号`)
;


-- 
-- TABLE: `驾驶人` 
--

ALTER TABLE `驾驶人` ADD CONSTRAINT `Ref驾驶员信息14` 
    FOREIGN KEY (`驾驶员编号`)
    REFERENCES `驾驶员信息`(`驾驶员编号`)
;

ALTER TABLE `驾驶人` ADD CONSTRAINT Refschedule15 
    FOREIGN KEY (`排班编号`)
    REFERENCES schedule(`排班编号`)
;


-- 
-- TABLE: `解缴记录` 
--

ALTER TABLE `解缴记录` ADD CONSTRAINT Refschedule20 
    FOREIGN KEY (`排班编号`)
    REFERENCES schedule(`排班编号`)
;


-- 
-- TABLE: `收入统计` 
--

ALTER TABLE `收入统计` ADD CONSTRAINT Refschedule19 
    FOREIGN KEY (`排班编号`)
    REFERENCES schedule(`排班编号`)
;


-- 
-- TABLE: `线路轨迹` 
--

ALTER TABLE `线路轨迹` ADD CONSTRAINT `Ref线路信息17` 
    FOREIGN KEY (`线路编号`)
    REFERENCES `线路信息`(`线路编号`)
;


