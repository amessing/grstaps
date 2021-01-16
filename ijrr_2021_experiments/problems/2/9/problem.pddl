(define (problem ijrr_9)
	(:domain ijrr)
	(:objects
		survivor0 survivor1 - survivor
		street30 street43 - street
		building10 building11 building20 building21 - building
	)
	(:init
		(atLocation survivor0 street30)
		(atLocation survivor1 building21)
		(onFire building11)
		(onFire building20)
		(underRubble street43)
		(underRubble building10)
		(needsRepair building10)
		(needsRepair building11)
		(needsRepair building20)
	)
	(:goal (and
		(atLocation survivor0 HOSPITAL)
		(atLocation survivor1 HOSPITAL)
		(not (underRubble street43))
		(not (needsRepair building10))
		(not (needsRepair building11))
		(not (needsRepair building20))
	))
	(:metric minimize total-time)
)
