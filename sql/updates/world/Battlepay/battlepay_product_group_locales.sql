/*
Navicat MySQL Data Transfer

Source Server         : LEGION BUILD
Source Server Version : 50640
Source Host           : localhost:3306
Source Database       : sakila

Target Server Type    : MYSQL
Target Server Version : 50640
File Encoding         : 65001

Date: 2026-08-13 04:59:35
*/

SET FOREIGN_KEY_CHECKS=0;

-- ----------------------------
-- Table structure for battlepay_product_group_locales
-- ----------------------------
DROP TABLE IF EXISTS `battlepay_product_group_locales`;
CREATE TABLE `battlepay_product_group_locales` (
  `GroupID` mediumint(8) unsigned NOT NULL DEFAULT '0',
  `Locale` text,
  `Name` text,
  PRIMARY KEY (`GroupID`) USING BTREE
) ENGINE=MyISAM DEFAULT CHARSET=utf8 ROW_FORMAT=DYNAMIC;

-- ----------------------------
-- Records of battlepay_product_group_locales
-- ----------------------------
