/*
Navicat MySQL Data Transfer

Source Server         : LEGION BUILD
Source Server Version : 50640
Source Host           : localhost:3306
Source Database       : sakila

Target Server Type    : MYSQL
Target Server Version : 50640
File Encoding         : 65001

Date: 2026-08-13 04:59:28
*/

SET FOREIGN_KEY_CHECKS=0;

-- ----------------------------
-- Table structure for battlepay_product_group
-- ----------------------------
DROP TABLE IF EXISTS `battlepay_product_group`;
CREATE TABLE `battlepay_product_group` (
  `GroupID` int(11) unsigned NOT NULL AUTO_INCREMENT,
  `Name` varchar(255) NOT NULL,
  `IconFileDataID` int(11) NOT NULL,
  `DisplayType` tinyint(3) unsigned NOT NULL,
  `Ordering` int(11) NOT NULL,
  `Flags` int(10) unsigned NOT NULL DEFAULT '0',
  `TokenType` tinyint(3) unsigned NOT NULL DEFAULT '0',
  `IngameOnly` tinyint(3) unsigned NOT NULL DEFAULT '0',
  `OwnsTokensOnly` tinyint(3) unsigned NOT NULL DEFAULT '0',
  PRIMARY KEY (`GroupID`) USING BTREE
) ENGINE=InnoDB AUTO_INCREMENT=15 DEFAULT CHARSET=utf8 ROW_FORMAT=DYNAMIC;

-- ----------------------------
-- Records of battlepay_product_group
-- ----------------------------
INSERT INTO `battlepay_product_group` VALUES ('1', 'Mount', '132261', '0', '1', '0', '1', '0', '0');
INSERT INTO `battlepay_product_group` VALUES ('2', 'Pets', '643856', '0', '2', '0', '1', '0', '0');
INSERT INTO `battlepay_product_group` VALUES ('3', 'Services', '1126583', '0', '3', '0', '1', '0', '0');
INSERT INTO `battlepay_product_group` VALUES ('4', 'Currencies', '133785', '0', '4', '0', '1', '0', '0');
INSERT INTO `battlepay_product_group` VALUES ('5', 'Professions', '1001623', '0', '5', '0', '1', '0', '0');
INSERT INTO `battlepay_product_group` VALUES ('7', 'Armors', '1030904', '0', '7', '0', '1', '0', '0');
INSERT INTO `battlepay_product_group` VALUES ('8', 'Weapons', '975736', '0', '9', '0', '1', '0', '0');
INSERT INTO `battlepay_product_group` VALUES ('9', 'Toys', '237429', '0', '10', '0', '1', '0', '0');
INSERT INTO `battlepay_product_group` VALUES ('10', 'Boosts', '1033988', '0', '11', '0', '1', '0', '0');
INSERT INTO `battlepay_product_group` VALUES ('11', 'Bags', '133628', '0', '12', '0', '1', '0', '0');
INSERT INTO `battlepay_product_group` VALUES ('12', 'Heirlooms', '1097737', '0', '8', '0', '1', '0', '0');
INSERT INTO `battlepay_product_group` VALUES ('13', 'Relics', '237244', '0', '13', '0', '1', '0', '0');
INSERT INTO `battlepay_product_group` VALUES ('14', 'Legendary', '133123', '0', '14', '0', '1', '0', '0');
