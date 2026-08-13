/*
Navicat MySQL Data Transfer

Source Server         : LEGION BUILD
Source Server Version : 50640
Source Host           : localhost:3306
Source Database       : sakila

Target Server Type    : MYSQL
Target Server Version : 50640
File Encoding         : 65001

Date: 2026-08-13 04:59:18
*/

SET FOREIGN_KEY_CHECKS=0;

-- ----------------------------
-- Table structure for battlepay_display_info_visuals
-- ----------------------------
DROP TABLE IF EXISTS `battlepay_display_info_visuals`;
CREATE TABLE `battlepay_display_info_visuals` (
  `DisplayInfoId` int(11) unsigned NOT NULL AUTO_INCREMENT,
  `DisplayId` int(11) unsigned NOT NULL DEFAULT '0',
  `VisualId` int(11) unsigned NOT NULL DEFAULT '0',
  `ProductName` varchar(1024) NOT NULL,
  UNIQUE KEY `DisplayInfoId` (`DisplayInfoId`) USING BTREE
) ENGINE=InnoDB DEFAULT CHARSET=utf8 ROW_FORMAT=DYNAMIC;

-- ----------------------------
-- Records of battlepay_display_info_visuals
-- ----------------------------
