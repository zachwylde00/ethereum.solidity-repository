.. index:: !mapping
.. _mapping-types:

Mapping Types
=============

Mapping types use the syntax ``mapping(_KeyType => _ValueType)`` and variables
are declared as a mapping type using the syntax ``mapping (_KeyType => _ValueType) _VariableModifiers _VariableName``.
The ``_KeyType`` can be any elementary type. This means it can be any of
the built-in value types plus ``bytes`` and ``string``. User-defined
or complex types like contract types, enums, mappings, structs and any array type
apart from ``bytes`` and ``string`` are not allowed.
``_ValueType`` can be any type, including mappings.

You can think of mappings as `hash tables <https://en.wikipedia.org/wiki/Hash_table>`_, which are virtually initialised
such that every possible key exists and is mapped to a value whose
byte-representation is all zeros, a type's :ref:`default value <default-value>`. The similarity ends there, the key data is not stored in a
mapping, only its ``keccak256`` hash is used to look up the value.

Because of this, mappings do not have a length or a concept of a key or
value being set.

Mappings can only have a data location of ``storage`` and thus
are allowed for state variables, as storage reference types
in functions, or as parameters for library functions.
They cannot be used as parameters or return parameters
of contract functions that are publicly visible.

You can mark state variables of mapping type as ``public`` and Solidity creates a
:ref:`getter <visibility-and-getters>` for you. The ``_KeyType`` becomes a parameter for the getter.
If ``_ValueType`` is a value type or a struct, the getter returns ``_ValueType``.
If ``_ValueType`` is an array or a mapping, the getter has one parameter for
each ``_KeyType``, recursively.

In the example below, the ``MappingExample`` contract defines a public ``balances``
mapping, with the key type an ``address``, and a value type a ``uint``, mapping
an Ethereum address to an unsigned integer value. As ``uint`` is a value type, the getter
returns a value that matches the type, which you can see in the ``MappingUser``
contract that returns the value at the specified address.

::

    pragma solidity >=0.4.0 <0.7.0;

    contract MappingExample {
        mapping(address => uint) public balances;

        function update(uint newBalance) public {
            balances[msg.sender] = newBalance;
        }
    }

    contract MappingUser {
        function f() public returns (uint) {
            MappingExample m = new MappingExample();
            m.update(100);
            return m.balances(address(this));
        }
    }

The example below is a simplified version of an `ERC20 token <https://github.com/OpenZeppelin/openzeppelin-contracts/blob/master/contracts/token/ERC20/ERC20.sol>`_.
``_allowances`` is an example of a mapping type inside another mapping type.
The example below uses ``_allowances`` to record the amount of a transaction and the two addresses it was between.

::

    pragma solidity >=0.4.0 <0.7.0;

    contract MappingExample {

        mapping (address => uint256) private _balances;
        mapping (address => mapping (address => uint256)) private _allowances;

        event Transfer(address indexed from, address indexed to, uint256 value);
        event Approval(address indexed owner, address indexed spender, uint256 value);

        function allowance(address owner, address spender) public view returns (uint256) {
            return _allowances[owner][spender];
        }

        function transferFrom(address sender, address recipient, uint256 amount) public returns (bool) {
            _transfer(sender, recipient, amount);
            _approve(sender, msg.sender, amount);
            return true;
        }

        function approve(address spender, uint256 amount) public returns (bool) {
            _approve(msg.sender, spender, amount);
            return true;
        }

        function _transfer(address sender, address recipient, uint256 amount) internal {
            require(sender != address(0), "ERC20: transfer from the zero address");
            require(recipient != address(0), "ERC20: transfer to the zero address");

            _balances[sender] -= amount;
            _balances[recipient] += amount;
            emit Transfer(sender, recipient, amount);
        }

        function _approve(address owner, address spender, uint256 amount) internal {
            require(owner != address(0), "ERC20: approve from the zero address");
            require(spender != address(0), "ERC20: approve to the zero address");

            _allowances[owner][spender] = amount;
            emit Approval(owner, spender, amount);
        }
    }

.. note::
  Mappings are not iterable, but it is possible to implement a data structure
  on top of them. For an example, see `iterable mapping <https://github.com/ethereum/dapp-bin/blob/master/library/iterable_mapping.sol>`_.
