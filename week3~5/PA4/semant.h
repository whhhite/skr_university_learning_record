#ifndef SEMANT_H_
#define SEMANT_H_

#include <assert.h>
#include <iostream>  
#include "cool-tree.h"
#include "stringtab.h"
#include "symtab.h"
#include "list.h"

#define TRUE 1
#define FALSE 0

//全局定义
enum Inheritable   {CanInherit, CantInherit};
enum Basicness     {Basic, NotBasic};
enum Reachability  {Reachable, UnReachable};
//此文件的major types(Enviroment,Classtable,InheritanceNode)互相交叉引用
//所以要首先声明
class Environment;
typedef Environment *EnvironmentP;
class ClassTable;
typedef ClassTable *ClassTableP;
class InheritanceNode;
typedef InheritanceNode *InheritanceNodeP;

//Cool里面的environment表达式可以被描述为如下:
//1. method绑定的symbol table
//2. attribute/local variable绑定的symbol table
//3. 所有类的symbol table
//4. 表达式中出现的类(SELF_TYPE类)
class Environment
{
 private:
    SymbolTable<Symbol, method_class> method_table;
    SymbolTable<Symbol, Entry>  var_table;
    ClassTableP class_table;
    Class_      self_class;

  public:
    Environment(ClassTableP ct, InheritanceNodeP sc);
    Environment(SymbolTable<Symbol, method_class> mt,
		            SymbolTable<Symbol, Entry>  vt,
		            ClassTableP ct,
		            InheritanceNodeP sc);
    EnvironmentP clone_Environment(InheritanceNodeP n);


    // class table operations
    //
    // report error的标准方法
    //
    ostream& semant_error();
    ostream& semant_error(tree_node *t);
    
    InheritanceNodeP lookup_class(Symbol s);

    // method table operations 
    void method_add(Symbol s, method_class *m);
    method_class *method_lookup(Symbol s);
    method_class *method_probe(Symbol s);
    void method_enterscope();
    void method_exitscope();

    // attribute table operations
    void var_add(Symbol s, Symbol typ);
    Symbol var_lookup(Symbol s);
    Symbol var_probe(Symbol s);
    void var_enterscope();
    void var_exitscope();

    // type operations
    Symbol get_self_type();
    int type_leq(Symbol subtype, Symbol supertype);
    Symbol type_lub(Symbol t1, Symbol t2);
};

// 继承图的一个节点是一个Cool里面的类,包括以下信息:
// 1. 继承图的父类
// 2. 孩子的list
// 3. 可以继承/不可以继承此类
// 4. 基本/非标准类
// 5. 这个类是否可以从Object继承关系中到达
// 6. a type check environment
class InheritanceNode : public class__class
{
 private:
  InheritanceNodeP parentnd;
  List<InheritanceNode> *children;
  Inheritable  inherit_status;
  Basicness    basic_status;
  Reachability reach_status;
  EnvironmentP env;
  
 public:
  InheritanceNode(Class_ c, Inheritable istatus, Basicness bstatus);
  int basic() { return (basic_status == Basic); }
  int inherit() { return (inherit_status == CanInherit); }
  void mark_reachable();
  int reachable() { return (reach_status == Reachable); }
  void add_child(InheritanceNodeP child);
  List<InheritanceNode> *get_children() { return children; }
  void set_parentnd(InheritanceNodeP p);
  InheritanceNodeP get_parentnd();
  //
  // 类X的type check environment 是通过复制X父类的environment和把X设立为Self class建立的
  //
  void copy_env(EnvironmentP e) { env = e->clone_Environment(this); }
  void build_feature_tables();
  //
  // 对于root Object class,讲创建一个全新的environment结构.
  //
  void init_env(ClassTableP ct);
  void type_check_features();
  void check_main_method();
  method_class *method_lookup(Symbol s) { return env->method_lookup(s); }
};

class ClassTable : public SymbolTable<Symbol, InheritanceNode>
{
//
//大部分的类型检查器都内置到类表的构造中
//
private:
  List<InheritanceNode> *nds;
  int semant_errors;             // counts the number of semantic errors
  void install_basic_classes();
  void install_class(InheritanceNodeP nd);
  void install_classes(Classes cs);
  void check_improper_inheritance();
  void build_inheritance_tree();
  void set_relations(InheritanceNodeP nd);
  void check_for_cycles();
  void build_feature_tables();
  void check_main();
  ostream& error_stream;

public:
  ClassTable(Classes);
  int errors() { return semant_errors; }
  InheritanceNodeP root();
  ostream& semant_error();
  ostream& semant_error(Class_ c);
  ostream& semant_error(Symbol filename, tree_node *t);
};


#endif

